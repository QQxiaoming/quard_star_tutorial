// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/core/channel.h"

#include <errno.h>
#include <sys/socket.h>

#include <algorithm>
#include <limits>
#include <memory>

#include "base/bind.h"
#include "base/containers/queue.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop_current.h"
#include "base/message_loop/message_pump_for_io.h"
#include "base/synchronization/lock.h"
#include "base/task_runner.h"
#include "build/build_config.h"
#include "mojo/core/core.h"
#include "mojo/public/cpp/platform/socket_utils_posix.h"

#if !defined(OS_NACL)
#include <sys/uio.h>
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include "mojo/core/mach_port_relay.h"
#endif

namespace mojo {
namespace core {

namespace {

const size_t kMaxBatchReadCapacity = 256 * 1024;

// A view over a Channel::Message object. The write queue uses these since
// large messages may need to be sent in chunks.
class MessageView {
 public:
  // Owns |message|. |offset| indexes the first unsent byte in the message.
  MessageView(Channel::MessagePtr message, size_t offset)
      : message_(std::move(message)),
        offset_(offset),
        handles_(message_->TakeHandlesForTransport()) {
    DCHECK_GT(message_->data_num_bytes(), offset_);
  }

  MessageView(MessageView&& other) { *this = std::move(other); }

  MessageView& operator=(MessageView&& other) {
    message_ = std::move(other.message_);
    offset_ = other.offset_;
    handles_ = std::move(other.handles_);
    return *this;
  }

  ~MessageView() {}

  const void* data() const {
    return static_cast<const char*>(message_->data()) + offset_;
  }

  size_t data_num_bytes() const { return message_->data_num_bytes() - offset_; }

  size_t data_offset() const { return offset_; }
  void advance_data_offset(size_t num_bytes) {
    DCHECK_GT(message_->data_num_bytes(), offset_ + num_bytes);
    offset_ += num_bytes;
  }

  std::vector<PlatformHandleInTransit> TakeHandles() {
    return std::move(handles_);
  }
  Channel::MessagePtr TakeMessage() { return std::move(message_); }

  void SetHandles(std::vector<PlatformHandleInTransit> handles) {
    handles_ = std::move(handles);
  }

 private:
  Channel::MessagePtr message_;
  size_t offset_;
  std::vector<PlatformHandleInTransit> handles_;

  DISALLOW_COPY_AND_ASSIGN(MessageView);
};

class ChannelPosix : public Channel,
#if defined(OS_MACOSX) && !defined(OS_IOS)
                     public MachPortRelay::Observer,
#endif
                     public base::MessageLoopCurrent::DestructionObserver,
                     public base::MessagePumpForIO::FdWatcher {
 public:
  ChannelPosix(Delegate* delegate,
               ConnectionParams connection_params,
               scoped_refptr<base::TaskRunner> io_task_runner)
      : Channel(delegate), self_(this), io_task_runner_(io_task_runner) {
    if (connection_params.server_endpoint().is_valid())
      server_ = connection_params.TakeServerEndpoint();
    else
      socket_ = connection_params.TakeEndpoint().TakePlatformHandle().TakeFD();

    CHECK(server_.is_valid() || socket_.is_valid());
  }

  void Start() override {
#if defined(OS_MACOSX) && !defined(OS_IOS)
    auto* relay = Core::Get()->GetMachPortRelay();
    if (relay) {
      // We should only have a relay if we know the remote process handle,
      // because that means we're in the broker process.
      relay->AddObserver(this);
    }
#endif

    if (io_task_runner_->RunsTasksInCurrentSequence()) {
      StartOnIOThread();
    } else {
      io_task_runner_->PostTask(
          FROM_HERE, base::BindOnce(&ChannelPosix::StartOnIOThread, this));
    }
  }

  void ShutDownImpl() override {
    // Always shut down asynchronously when called through the public interface.
    io_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&ChannelPosix::ShutDownOnIOThread, this));
  }

  void Write(MessagePtr message) override {
#if defined(OS_MACOSX) && !defined(OS_IOS)
    // If this message has Mach ports and we have a MachPortRelay, use the relay
    // to rewrite the ports as receive rights from which the send right can be
    // read. See |MachPortRelay::SendPortsToProcess()|.
    //
    // Note that if we don't have a relay, the receiving process must, and they
    // must also have the ability to extract a send right from the ports that
    // are already attached.
    MachPortRelay* relay = Core::Get()->GetMachPortRelay();
    if (relay && remote_process().is_valid() && message->has_mach_ports()) {
      if (relay->port_provider()->TaskForPid(remote_process().get()) ==
          MACH_PORT_NULL) {
        // We also need to have a task port for the remote process before we can
        // send it any other ports. If we don't have one yet, queue the message
        // until OnProcessReady() is invoked.
        base::AutoLock lock(task_port_wait_lock_);
        pending_outgoing_with_mach_ports_.emplace_back(std::move(message));
        return;
      }

      relay->SendPortsToProcess(message.get(), remote_process().get());
    }
#endif

    bool write_error = false;
    {
      base::AutoLock lock(write_lock_);
      if (reject_writes_)
        return;
      if (outgoing_messages_.empty()) {
        if (!WriteNoLock(MessageView(std::move(message), 0)))
          reject_writes_ = write_error = true;
      } else {
        outgoing_messages_.emplace_back(std::move(message), 0);
      }
    }
    if (write_error) {
      // Invoke OnWriteError() asynchronously on the IO thread, in case Write()
      // was called by the delegate, in which case we should not re-enter it.
      io_task_runner_->PostTask(
          FROM_HERE, base::BindOnce(&ChannelPosix::OnWriteError, this,
                                    Error::kDisconnected));
    }
  }

  void LeakHandle() override {
    DCHECK(io_task_runner_->RunsTasksInCurrentSequence());
    leak_handle_ = true;
  }

  bool GetReadPlatformHandles(const void* payload,
                              size_t payload_size,
                              size_t num_handles,
                              const void* extra_header,
                              size_t extra_header_size,
                              std::vector<PlatformHandle>* handles,
                              bool* deferred) override {
    if (num_handles > std::numeric_limits<uint16_t>::max())
      return false;
#if defined(OS_MACOSX) && !defined(OS_IOS)
    // On OSX, we can have mach ports which are located in the extra header
    // section.
    using MachPortsEntry = Channel::Message::MachPortsEntry;
    using MachPortsExtraHeader = Channel::Message::MachPortsExtraHeader;
    if (extra_header_size <
        sizeof(MachPortsExtraHeader) + num_handles * sizeof(MachPortsEntry)) {
      return false;
    }
    const MachPortsExtraHeader* mach_ports_header =
        reinterpret_cast<const MachPortsExtraHeader*>(extra_header);
    size_t num_mach_ports = mach_ports_header->num_ports;
    if (num_mach_ports > num_handles)
      return false;
    if (incoming_fds_.size() + num_mach_ports < num_handles)
      return true;

    std::vector<PlatformHandleInTransit> handles_in_transit(num_handles);
    const MachPortsEntry* mach_ports = mach_ports_header->entries;

    // If we know the remote process handle, we assume all incoming Mach ports
    // are send right references owned by the remote process. Otherwise they're
    // receive ports we can use to read a send right.
    const bool extract_send_rights = remote_process().is_valid();
    for (size_t i = 0, mach_port_index = 0; i < num_handles; ++i) {
      if (mach_port_index < num_mach_ports &&
          mach_ports[mach_port_index].index == i) {
        mach_port_t port_name =
            static_cast<mach_port_t>(mach_ports[mach_port_index].mach_port);
        if (extract_send_rights) {
          handles_in_transit[i] =
              PlatformHandleInTransit::CreateForMachPortName(port_name);
        } else {
          handles_in_transit[i] = PlatformHandleInTransit(
              PlatformHandle(MachPortRelay::ReceiveSendRight(
                  base::mac::ScopedMachReceiveRight(port_name))));
        }
        mach_port_index++;
      } else {
        if (incoming_fds_.empty())
          return false;
        handles_in_transit[i] = PlatformHandleInTransit(
            PlatformHandle(std::move(incoming_fds_.front())));
        incoming_fds_.pop_front();
      }
    }
    if (extract_send_rights && num_mach_ports) {
      MachPortRelay* relay = Core::Get()->GetMachPortRelay();
      DCHECK(relay);
      // Extracting send rights requires that we have a task port for the
      // remote process, which we may not yet have.
      if (relay->port_provider()->TaskForPid(remote_process().get()) !=
          MACH_PORT_NULL) {
        // We do have a task port, so extract the send rights immediately.
        for (auto& handle : handles_in_transit) {
          if (handle.is_mach_port_name()) {
            handle = PlatformHandleInTransit(PlatformHandle(relay->ExtractPort(
                handle.mach_port_name(), remote_process().get())));
          }
        }
      } else {
        // No task port, we have to defer this message.
        *deferred = true;
        base::AutoLock lock(task_port_wait_lock_);
        std::vector<uint8_t> data(payload_size);
        memcpy(data.data(), payload, payload_size);
        pending_incoming_with_mach_ports_.emplace_back(
            std::move(data), std::move(handles_in_transit));
        return true;
      }
    }

    handles->resize(handles_in_transit.size());
    for (size_t i = 0; i < handles->size(); ++i)
      handles->at(i) = handles_in_transit[i].TakeHandle();
#else
    if (incoming_fds_.size() < num_handles)
      return true;

    handles->resize(num_handles);
    for (size_t i = 0; i < num_handles; ++i) {
      handles->at(i) = PlatformHandle(std::move(incoming_fds_.front()));
      incoming_fds_.pop_front();
    }
#endif

    return true;
  }

 private:
  ~ChannelPosix() override {
    DCHECK(!read_watcher_);
    DCHECK(!write_watcher_);
  }

  void StartOnIOThread() {
    DCHECK(!read_watcher_);
    DCHECK(!write_watcher_);
    read_watcher_.reset(
        new base::MessagePumpForIO::FdWatchController(FROM_HERE));
    base::MessageLoopCurrent::Get()->AddDestructionObserver(this);
    if (server_.is_valid()) {
      base::MessageLoopCurrentForIO::Get()->WatchFileDescriptor(
          server_.platform_handle().GetFD().get(), false /* persistent */,
          base::MessagePumpForIO::WATCH_READ, read_watcher_.get(), this);
    } else {
      write_watcher_.reset(
          new base::MessagePumpForIO::FdWatchController(FROM_HERE));
      base::MessageLoopCurrentForIO::Get()->WatchFileDescriptor(
          socket_.get(), true /* persistent */,
          base::MessagePumpForIO::WATCH_READ, read_watcher_.get(), this);
      base::AutoLock lock(write_lock_);
      FlushOutgoingMessagesNoLock();
    }
  }

  void WaitForWriteOnIOThread() {
    base::AutoLock lock(write_lock_);
    WaitForWriteOnIOThreadNoLock();
  }

  void WaitForWriteOnIOThreadNoLock() {
    if (pending_write_)
      return;
    if (!write_watcher_)
      return;
    if (io_task_runner_->RunsTasksInCurrentSequence()) {
      pending_write_ = true;
      base::MessageLoopCurrentForIO::Get()->WatchFileDescriptor(
          socket_.get(), false /* persistent */,
          base::MessagePumpForIO::WATCH_WRITE, write_watcher_.get(), this);
    } else {
      io_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(&ChannelPosix::WaitForWriteOnIOThread, this));
    }
  }

  void ShutDownOnIOThread() {
    base::MessageLoopCurrent::Get()->RemoveDestructionObserver(this);

    read_watcher_.reset();
    write_watcher_.reset();
    if (leak_handle_) {
      ignore_result(socket_.release());
      server_.TakePlatformHandle().release();
    } else {
      socket_.reset();
      ignore_result(server_.TakePlatformHandle());
    }
#if defined(OS_MACOSX)
    fds_to_close_.clear();
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
    auto* relay = Core::Get()->GetMachPortRelay();
    if (relay)
      relay->RemoveObserver(this);
#endif

    // May destroy the |this| if it was the last reference.
    self_ = nullptr;
  }

#if defined(OS_MACOSX) && !defined(OS_IOS)
  // MachPortRelay::Observer:
  void OnProcessReady(base::ProcessHandle process) override {
    if (process != remote_process().get())
      return;

    io_task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(
            &ChannelPosix::FlushPendingMessagesWithMachPortsOnIOThread, this));
  }

  void FlushPendingMessagesWithMachPortsOnIOThread() {
    // We have a task port for the remote process. Now we can send or accept
    // any pending messages with Mach ports.
    std::vector<RawIncomingMessage> incoming;
    std::vector<MessagePtr> outgoing;
    {
      base::AutoLock lock(task_port_wait_lock_);
      if (reject_incoming_messages_with_mach_ports_)
        return;
      std::swap(pending_incoming_with_mach_ports_, incoming);
      std::swap(pending_outgoing_with_mach_ports_, outgoing);
    }

    DCHECK(remote_process().is_valid());
    base::ProcessHandle process = remote_process().get();
    MachPortRelay* relay = Core::Get()->GetMachPortRelay();
    DCHECK(relay);
    for (auto& message : incoming) {
      Channel::Delegate* d = delegate();
      if (!d)
        break;
      std::vector<PlatformHandle> handles(message.handles.size());
      for (size_t i = 0; i < message.handles.size(); ++i) {
        if (message.handles[i].is_mach_port_name()) {
          handles[i] = PlatformHandle(
              relay->ExtractPort(message.handles[i].mach_port_name(), process));
        } else {
          DCHECK(!message.handles[i].owning_process().is_valid());
          handles[i] = message.handles[i].TakeHandle();
        }
      }
      d->OnChannelMessage(message.data.data(), message.data.size(),
                          std::move(handles));
    }

    for (auto& message : outgoing) {
      relay->SendPortsToProcess(message.get(), process);
      Write(std::move(message));
    }
  }
#endif

  // base::MessageLoopCurrent::DestructionObserver:
  void WillDestroyCurrentMessageLoop() override {
    DCHECK(io_task_runner_->RunsTasksInCurrentSequence());
    if (self_)
      ShutDownOnIOThread();
  }

  // base::MessagePumpForIO::FdWatcher:
  void OnFileCanReadWithoutBlocking(int fd) override {
    if (server_.is_valid()) {
      CHECK_EQ(fd, server_.platform_handle().GetFD().get());
#if !defined(OS_NACL)
      read_watcher_.reset();
      base::MessageLoopCurrent::Get()->RemoveDestructionObserver(this);

      AcceptSocketConnection(server_.platform_handle().GetFD().get(), &socket_);
      ignore_result(server_.TakePlatformHandle());
      if (!socket_.is_valid()) {
        OnError(Error::kConnectionFailed);
        return;
      }
      StartOnIOThread();
#else
      NOTREACHED();
#endif
      return;
    }
    CHECK_EQ(fd, socket_.get());

    bool validation_error = false;
    bool read_error = false;
    size_t next_read_size = 0;
    size_t buffer_capacity = 0;
    size_t total_bytes_read = 0;
    size_t bytes_read = 0;
    do {
      buffer_capacity = next_read_size;
      char* buffer = GetReadBuffer(&buffer_capacity);
      DCHECK_GT(buffer_capacity, 0u);

      std::vector<base::ScopedFD> incoming_fds;
      ssize_t read_result =
          SocketRecvmsg(socket_.get(), buffer, buffer_capacity, &incoming_fds);
      for (auto& fd : incoming_fds)
        incoming_fds_.emplace_back(std::move(fd));

      if (read_result > 0) {
        bytes_read = static_cast<size_t>(read_result);
        total_bytes_read += bytes_read;
        if (!OnReadComplete(bytes_read, &next_read_size)) {
          read_error = true;
          validation_error = true;
          break;
        }
      } else if (read_result == 0 ||
                 (errno != EAGAIN && errno != EWOULDBLOCK)) {
        read_error = true;
        break;
      }
    } while (bytes_read == buffer_capacity &&
             total_bytes_read < kMaxBatchReadCapacity && next_read_size > 0);
    if (read_error) {
      // Stop receiving read notifications.
      read_watcher_.reset();
      if (validation_error)
        OnError(Error::kReceivedMalformedData);
      else
        OnError(Error::kDisconnected);
    }
  }

  void OnFileCanWriteWithoutBlocking(int fd) override {
    bool write_error = false;
    {
      base::AutoLock lock(write_lock_);
      pending_write_ = false;
      if (!FlushOutgoingMessagesNoLock())
        reject_writes_ = write_error = true;
    }
    if (write_error)
      OnWriteError(Error::kDisconnected);
  }

  // Attempts to write a message directly to the channel. If the full message
  // cannot be written, it's queued and a wait is initiated to write the message
  // ASAP on the I/O thread.
  bool WriteNoLock(MessageView message_view) {
    if (server_.is_valid()) {
      outgoing_messages_.emplace_front(std::move(message_view));
      return true;
    }
    size_t bytes_written = 0;
    do {
      message_view.advance_data_offset(bytes_written);

      ssize_t result;
      std::vector<PlatformHandleInTransit> handles = message_view.TakeHandles();
      if (!handles.empty()) {
        iovec iov = {const_cast<void*>(message_view.data()),
                     message_view.data_num_bytes()};
        std::vector<base::ScopedFD> fds(handles.size());
        for (size_t i = 0; i < handles.size(); ++i)
          fds[i] = handles[i].TakeHandle().TakeFD();
        // TODO: Handle lots of handles.
        result = SendmsgWithHandles(socket_.get(), &iov, 1, fds);
        if (result >= 0) {
#if defined(OS_MACOSX)
          // There is a bug on OSX which makes it dangerous to close
          // a file descriptor while it is in transit. So instead we
          // store the file descriptor in a set and send a message to
          // the recipient, which is queued AFTER the message that
          // sent the FD. The recipient will reply to the message,
          // letting us know that it is now safe to close the file
          // descriptor. For more information, see:
          // http://crbug.com/298276
          MessagePtr fds_message(
              new Channel::Message(sizeof(fds[0]) * fds.size(), 0,
                                   Message::MessageType::HANDLES_SENT));
          memcpy(fds_message->mutable_payload(), fds.data(),
                 sizeof(fds[0]) * fds.size());
          outgoing_messages_.emplace_back(std::move(fds_message), 0);
          {
            base::AutoLock l(fds_to_close_lock_);
            for (auto& fd : fds)
              fds_to_close_.emplace_back(std::move(fd));
          }
#endif  // defined(OS_MACOSX)
        } else {
          // Message transmission failed, so pull the FDs back into |handles|
          // so they can be held by the Message again.
          for (size_t i = 0; i < fds.size(); ++i) {
            handles[i] =
                PlatformHandleInTransit(PlatformHandle(std::move(fds[i])));
          }
        }
      } else {
        result = SocketWrite(socket_.get(), message_view.data(),
                             message_view.data_num_bytes());
      }

      if (result < 0) {
        if (errno != EAGAIN &&
            errno != EWOULDBLOCK
#if defined(OS_MACOSX)
            // On OS X if sendmsg() is trying to send fds between processes and
            // there isn't enough room in the output buffer to send the fd
            // structure over atomically then EMSGSIZE is returned.
            //
            // EMSGSIZE presents a problem since the system APIs can only call
            // us when there's room in the socket buffer and not when there is
            // "enough" room.
            //
            // The current behavior is to return to the event loop when EMSGSIZE
            // is received and hopefull service another FD.  This is however
            // still technically a busy wait since the event loop will call us
            // right back until the receiver has read enough data to allow
            // passing the FD over atomically.
            && errno != EMSGSIZE
#endif
            ) {
          return false;
        }
        message_view.SetHandles(std::move(handles));
        outgoing_messages_.emplace_front(std::move(message_view));
        WaitForWriteOnIOThreadNoLock();
        return true;
      }

      bytes_written = static_cast<size_t>(result);
    } while (bytes_written < message_view.data_num_bytes());

    return FlushOutgoingMessagesNoLock();
  }

  bool FlushOutgoingMessagesNoLock() {
    base::circular_deque<MessageView> messages;
    std::swap(outgoing_messages_, messages);

    while (!messages.empty()) {
      if (!WriteNoLock(std::move(messages.front())))
        return false;

      messages.pop_front();
      if (!outgoing_messages_.empty()) {
        // The message was requeued by WriteNoLock(), so we have to wait for
        // pipe to become writable again. Repopulate the message queue and exit.
        // If sending the message triggered any control messages, they may be
        // in |outgoing_messages_| in addition to or instead of the message
        // being sent.
        std::swap(messages, outgoing_messages_);
        while (!messages.empty()) {
          outgoing_messages_.push_front(std::move(messages.back()));
          messages.pop_back();
        }
        return true;
      }
    }

    return true;
  }

#if defined(OS_MACOSX)
  bool OnControlMessage(Message::MessageType message_type,
                        const void* payload,
                        size_t payload_size,
                        std::vector<PlatformHandle> handles) override {
    switch (message_type) {
      case Message::MessageType::HANDLES_SENT: {
        if (payload_size == 0)
          break;
        MessagePtr message(new Channel::Message(
            payload_size, 0, Message::MessageType::HANDLES_SENT_ACK));
        memcpy(message->mutable_payload(), payload, payload_size);
        Write(std::move(message));
        return true;
      }

      case Message::MessageType::HANDLES_SENT_ACK: {
        size_t num_fds = payload_size / sizeof(int);
        if (num_fds == 0 || payload_size % sizeof(int) != 0)
          break;

        const int* fds = reinterpret_cast<const int*>(payload);
        if (!CloseHandles(fds, num_fds))
          break;
        return true;
      }

      default:
        break;
    }

    return false;
  }

  // Closes handles referenced by |fds|. Returns false if |num_fds| is 0, or if
  // |fds| does not match a sequence of handles in |fds_to_close_|.
  bool CloseHandles(const int* fds, size_t num_fds) {
    base::AutoLock l(fds_to_close_lock_);
    if (!num_fds)
      return false;

    auto start = std::find_if(
        fds_to_close_.begin(), fds_to_close_.end(),
        [&fds](const base::ScopedFD& fd) { return fd.get() == fds[0]; });
    if (start == fds_to_close_.end())
      return false;

    auto it = start;
    size_t i = 0;
    // The FDs in the message should match a sequence of handles in
    // |fds_to_close_|.
    // TODO(wez): Consider making |fds_to_close_| a circular_deque<>
    // for greater efficiency? Or assign a unique Id to each FD-containing
    // message, and map that to a vector of FDs to close, to avoid the
    // need for this traversal? Id could even be the first FD in the message.
    for (; i < num_fds && it != fds_to_close_.end(); i++, ++it) {
      if (it->get() != fds[i])
        return false;
    }
    if (i != num_fds)
      return false;

    // Close the FDs by erase()ing their ScopedFDs.
    fds_to_close_.erase(start, it);
    return true;
  }
#endif  // defined(OS_MACOSX)

  void OnWriteError(Error error) {
    DCHECK(io_task_runner_->RunsTasksInCurrentSequence());
    DCHECK(reject_writes_);

    if (error == Error::kDisconnected) {
      // If we can't write because the pipe is disconnected then continue
      // reading to fetch any in-flight messages, relying on end-of-stream to
      // signal the actual disconnection.
      if (read_watcher_) {
        write_watcher_.reset();
        return;
      }
    }

    OnError(error);
  }

  // Keeps the Channel alive at least until explicit shutdown on the IO thread.
  scoped_refptr<Channel> self_;

  // We may be initialized with a server socket, in which case this will be
  // valid until it accepts an incoming connection.
  PlatformChannelServerEndpoint server_;

  // The socket over which to communicate. May be passed in at construction time
  // or accepted over |server_|.
  base::ScopedFD socket_;

  scoped_refptr<base::TaskRunner> io_task_runner_;

  // These watchers must only be accessed on the IO thread.
  std::unique_ptr<base::MessagePumpForIO::FdWatchController> read_watcher_;
  std::unique_ptr<base::MessagePumpForIO::FdWatchController> write_watcher_;

  base::circular_deque<base::ScopedFD> incoming_fds_;

  // Protects |pending_write_| and |outgoing_messages_|.
  base::Lock write_lock_;
  bool pending_write_ = false;
  bool reject_writes_ = false;
  base::circular_deque<MessageView> outgoing_messages_;

  bool leak_handle_ = false;

#if defined(OS_MACOSX)
  base::Lock fds_to_close_lock_;
  std::vector<base::ScopedFD> fds_to_close_;
#if !defined(OS_IOS)
  // Guards access to the send/receive queues below. These are messages that
  // can't be fully accepted from or dispatched to the Channel user yet because
  // we're still waiting on a task port for the remote process.
  struct RawIncomingMessage {
    RawIncomingMessage(std::vector<uint8_t> data,
                       std::vector<PlatformHandleInTransit> handles)
        : data(std::move(data)), handles(std::move(handles)) {}
    RawIncomingMessage(RawIncomingMessage&&) = default;
    ~RawIncomingMessage() = default;

    std::vector<uint8_t> data;
    std::vector<PlatformHandleInTransit> handles;
  };

  base::Lock task_port_wait_lock_;
  bool reject_incoming_messages_with_mach_ports_ = false;
  std::vector<MessagePtr> pending_outgoing_with_mach_ports_;
  std::vector<RawIncomingMessage> pending_incoming_with_mach_ports_;
#endif  // !defined(OS_IOS)
#endif  // defined(OS_MACOSX)

  DISALLOW_COPY_AND_ASSIGN(ChannelPosix);
};

}  // namespace

// static
scoped_refptr<Channel> Channel::Create(
    Delegate* delegate,
    ConnectionParams connection_params,
    scoped_refptr<base::TaskRunner> io_task_runner) {
  return new ChannelPosix(delegate, std::move(connection_params),
                          io_task_runner);
}

}  // namespace core
}  // namespace mojo
