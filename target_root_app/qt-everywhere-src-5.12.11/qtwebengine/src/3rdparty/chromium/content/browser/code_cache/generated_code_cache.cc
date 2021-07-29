// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/code_cache/generated_code_cache.h"
#include "base/memory/ptr_util.h"
#include "net/base/completion_callback.h"
#include "net/base/completion_once_callback.h"
#include "net/http/http_util.h"

namespace content {

namespace {
// Checks if |requesting_origin| is allowed to cache code for |resource_url|.
//   |resource_url| is the url corresponding to the requested resource.
//   If this url is invalid we don't cache the code.
//   |requesting_origin| is the origin that has requested the resource.
//   If this is a unique origin, then we don't cache the code.
// For example, if http://script.com/script1.js is requested by
// http://example.com, then http://script.com/script.js is the resource_url
// and example.com is the requesting_origin.
bool IsAllowedToCache(const GURL& resource_url,
                      const url::Origin& requesting_origin) {
  // Don't cache the code corresponding to unique origins. The same-origin
  // checks should always fail for unique origins but the serialized value of
  // unique origins does not ensure this.
  if (requesting_origin.unique())
    return false;

  // If the resource url or requesting url is invalid don't cache the code.
  if (!resource_url.is_valid())
    return false;

  return true;
}

// Generates the cache key for the given |resource_url| and the
// |requesting_origin|. This returns the key by concatenating the
// serialized url and origin with a separator in between.
std::string GetCacheKey(const GURL& resource_url,
                        const url::Origin& requesting_origin) {
  DCHECK(!requesting_origin.unique());
  DCHECK(resource_url.is_valid());
  // Add a prefix _ so it can't be parsed as a valid URL.
  std::string key = "_key";
  // Remove reference, username and password sections of the URL.
  key.append(net::HttpUtil::SpecForRequest(resource_url));
  // Add a separator between URL and origin to avoid any possibility of
  // attacks by crafting the URL. URLs do not contain any control ASCII
  // characters, and also space is encoded. So use ' \n' as a seperator.
  key.append(" \n");
  key.append(requesting_origin.Serialize());
  return key;
}
}  // namespace

// Stores the information about a pending request while disk backend is
// being initialized.
class GeneratedCodeCache::PendingOperation {
 public:
  PendingOperation(Operation op,
                   std::string key,
                   scoped_refptr<net::IOBufferWithSize>);
  PendingOperation(Operation op, std::string key, const ReadDataCallback&);
  PendingOperation(Operation op, std::string key);

  ~PendingOperation();

  Operation operation() const { return op_; }
  const std::string& key() const { return key_; }
  const scoped_refptr<net::IOBufferWithSize> data() const { return data_; }
  const ReadDataCallback& callback() const { return callback_; }

 private:
  const Operation op_;
  const std::string key_;
  const scoped_refptr<net::IOBufferWithSize> data_;
  const ReadDataCallback callback_;
};

GeneratedCodeCache::PendingOperation::PendingOperation(
    Operation op,
    std::string key,
    scoped_refptr<net::IOBufferWithSize> buffer)
    : op_(op),
      key_(std::move(key)),
      data_(buffer),
      callback_(ReadDataCallback()) {}

GeneratedCodeCache::PendingOperation::PendingOperation(
    Operation op,
    std::string key,
    const ReadDataCallback& callback)
    : op_(op),
      key_(std::move(key)),
      data_(scoped_refptr<net::IOBufferWithSize>()),
      callback_(callback) {}

GeneratedCodeCache::PendingOperation::PendingOperation(Operation op,
                                                       std::string key)
    : op_(op),
      key_(std::move(key)),
      data_(scoped_refptr<net::IOBufferWithSize>()),
      callback_(ReadDataCallback()) {}

GeneratedCodeCache::PendingOperation::~PendingOperation() = default;

// Static factory method.
std::unique_ptr<GeneratedCodeCache> GeneratedCodeCache::Create(
    const base::FilePath& path,
    int max_size_bytes) {
  return base::WrapUnique(new GeneratedCodeCache(path, max_size_bytes));
}

GeneratedCodeCache::~GeneratedCodeCache() = default;

void GeneratedCodeCache::WriteData(
    const GURL& url,
    const url::Origin& origin,
    scoped_refptr<net::IOBufferWithSize> buffer) {
  // Silently ignore the requests.
  if (backend_state_ == kFailed)
    return;

  // If the url is invalid or if it is from a unique origin, we should not
  // cache the code.
  if (!IsAllowedToCache(url, origin))
    return;

  std::string key = GetCacheKey(url, origin);
  if (backend_state_ != kInitialized) {
    // Insert it into the list of pending operations while the backend is
    // still being opened.
    pending_ops_.push_back(std::make_unique<PendingOperation>(
        Operation::kWrite, std::move(key), buffer));
    return;
  }

  WriteDataImpl(key, buffer);
}

void GeneratedCodeCache::FetchEntry(const GURL& url,
                                    const url::Origin& origin,
                                    ReadDataCallback read_data_callback) {
  if (backend_state_ == kFailed) {
    // Silently ignore the requests.
    std::move(read_data_callback).Run(scoped_refptr<net::IOBufferWithSize>());
    return;
  }

  // If the url is invalid or if it is from a unique origin, we should not
  // cache the code.
  if (!IsAllowedToCache(url, origin)) {
    std::move(read_data_callback).Run(scoped_refptr<net::IOBufferWithSize>());
    return;
  }

  std::string key = GetCacheKey(url, origin);
  if (backend_state_ != kInitialized) {
    // Insert it into the list of pending operations while the backend is
    // still being opened.
    pending_ops_.push_back(std::make_unique<PendingOperation>(
        Operation::kFetch, std::move(key), read_data_callback));
    return;
  }

  FetchEntryImpl(key, read_data_callback);
}

void GeneratedCodeCache::DeleteEntry(const GURL& url,
                                     const url::Origin& origin) {
  // Silently ignore the requests.
  if (backend_state_ == kFailed)
    return;

  // If the url is invalid or if it is from a unique origin, we should not
  // cache the code.
  if (!IsAllowedToCache(url, origin))
    return;

  std::string key = GetCacheKey(url, origin);
  if (backend_state_ != kInitialized) {
    // Insert it into the list of pending operations while the backend is
    // still being opened.
    pending_ops_.push_back(
        std::make_unique<PendingOperation>(Operation::kDelete, std::move(key)));
    return;
  }

  DeleteEntryImpl(key);
}

GeneratedCodeCache::GeneratedCodeCache(const base::FilePath& path,
                                       int max_size_bytes)
    : backend_state_(kUnInitialized),
      path_(path),
      max_size_bytes_(max_size_bytes),
      weak_ptr_factory_(this) {
  CreateBackend();
}

void GeneratedCodeCache::CreateBackend() {
  // Create a new Backend pointer that cleans itself if the GeneratedCodeCache
  // instance is not live when the CreateCacheBackend finishes.
  scoped_refptr<base::RefCountedData<ScopedBackendPtr>> shared_backend_ptr =
      new base::RefCountedData<ScopedBackendPtr>();

  net::CompletionOnceCallback create_backend_complete =
      base::BindOnce(&GeneratedCodeCache::DidCreateBackend,
                     weak_ptr_factory_.GetWeakPtr(), shared_backend_ptr);

  // If the initialization of the existing cache fails, this call would delete
  // all the contents and recreates a new one.
  int rv = disk_cache::CreateCacheBackend(
      net::GENERATED_CODE_CACHE, net::CACHE_BACKEND_SIMPLE, path_,
      max_size_bytes_, true, nullptr, &shared_backend_ptr->data,
      std::move(create_backend_complete));
  if (rv != net::ERR_IO_PENDING) {
    DidCreateBackend(shared_backend_ptr, rv);
  }
}

void GeneratedCodeCache::DidCreateBackend(
    scoped_refptr<base::RefCountedData<ScopedBackendPtr>> backend_ptr,
    int rv) {
  if (rv != net::OK) {
    backend_state_ = kFailed;
    // Process pending operations to process any required callbacks.
    IssuePendingOperations();
    return;
  }

  backend_ = std::move(backend_ptr->data);
  backend_state_ = kInitialized;
  IssuePendingOperations();
}

void GeneratedCodeCache::IssuePendingOperations() {
  DCHECK_EQ(backend_state_, kInitialized);
  // Issue all the pending operations that were received when creating
  // the backend.
  for (auto const& op : pending_ops_) {
    switch (op->operation()) {
      case kFetch:
        FetchEntryImpl(op->key(), op->callback());
        break;
      case kWrite:
        WriteDataImpl(op->key(), op->data());
        break;
      case kDelete:
        DeleteEntryImpl(op->key());
        break;
    }
  }
  pending_ops_.clear();
}

void GeneratedCodeCache::WriteDataImpl(
    const std::string& key,
    scoped_refptr<net::IOBufferWithSize> buffer) {
  if (backend_state_ != kInitialized)
    return;

  scoped_refptr<base::RefCountedData<disk_cache::Entry*>> entry_ptr =
      new base::RefCountedData<disk_cache::Entry*>();
  net::CompletionOnceCallback callback =
      base::BindOnce(&GeneratedCodeCache::OpenCompleteForWriteData,
                     weak_ptr_factory_.GetWeakPtr(), buffer, key, entry_ptr);

  int result =
      backend_->OpenEntry(key, net::LOW, &entry_ptr->data, std::move(callback));
  if (result != net::ERR_IO_PENDING) {
    OpenCompleteForWriteData(buffer, key, entry_ptr, result);
  }
}

void GeneratedCodeCache::OpenCompleteForWriteData(
    scoped_refptr<net::IOBufferWithSize> buffer,
    const std::string& key,
    scoped_refptr<base::RefCountedData<disk_cache::Entry*>> entry,
    int rv) {
  if (rv != net::OK) {
    net::CompletionOnceCallback callback =
        base::BindOnce(&GeneratedCodeCache::CreateCompleteForWriteData,
                       weak_ptr_factory_.GetWeakPtr(), buffer, entry);

    int result =
        backend_->CreateEntry(key, net::LOW, &entry->data, std::move(callback));
    if (result != net::ERR_IO_PENDING) {
      CreateCompleteForWriteData(buffer, entry, result);
    }
    return;
  }

  DCHECK(entry->data);
  disk_cache::ScopedEntryPtr disk_entry(entry->data);

  // This call will truncate the data. This is safe to do since we read the
  // entire data at the same time currently. If we want to read in parts we have
  // to doom the entry first.
  disk_entry->WriteData(kDataIndex, 0, buffer.get(), buffer->size(),
                        net::CompletionOnceCallback(), true);
}

void GeneratedCodeCache::CreateCompleteForWriteData(
    scoped_refptr<net::IOBufferWithSize> buffer,
    scoped_refptr<base::RefCountedData<disk_cache::Entry*>> entry,
    int rv) {
  if (rv != net::OK)
    return;

  DCHECK(entry->data);
  disk_cache::ScopedEntryPtr disk_entry(entry->data);
  disk_entry->WriteData(kDataIndex, 0, buffer.get(), buffer->size(),
                        net::CompletionOnceCallback(), true);
}

void GeneratedCodeCache::FetchEntryImpl(const std::string& key,
                                        ReadDataCallback read_data_callback) {
  if (backend_state_ != kInitialized) {
    std::move(read_data_callback).Run(scoped_refptr<net::IOBufferWithSize>());
    return;
  }

  scoped_refptr<base::RefCountedData<disk_cache::Entry*>> entry_ptr =
      new base::RefCountedData<disk_cache::Entry*>();

  net::CompletionOnceCallback callback = base::BindOnce(
      &GeneratedCodeCache::OpenCompleteForReadData,
      weak_ptr_factory_.GetWeakPtr(), read_data_callback, entry_ptr);

  // This is a part of loading cycle and hence should run with a high priority.
  int result = backend_->OpenEntry(key, net::HIGHEST, &entry_ptr->data,
                                   std::move(callback));
  if (result != net::ERR_IO_PENDING) {
    OpenCompleteForReadData(read_data_callback, entry_ptr, result);
  }
}

void GeneratedCodeCache::OpenCompleteForReadData(
    ReadDataCallback read_data_callback,
    scoped_refptr<base::RefCountedData<disk_cache::Entry*>> entry,
    int rv) {
  if (rv != net::OK) {
    std::move(read_data_callback).Run(scoped_refptr<net::IOBufferWithSize>());
    return;
  }

  // There should be a valid entry if the open was successful.
  DCHECK(entry->data);

  disk_cache::ScopedEntryPtr disk_entry(entry->data);
  int size = disk_entry->GetDataSize(kDataIndex);
  scoped_refptr<net::IOBufferWithSize> buffer(new net::IOBufferWithSize(size));
  net::CompletionOnceCallback callback = base::BindOnce(
      &GeneratedCodeCache::ReadDataComplete, weak_ptr_factory_.GetWeakPtr(),
      read_data_callback, buffer);
  int result = disk_entry->ReadData(kDataIndex, 0, buffer.get(), size,
                                    std::move(callback));
  if (result != net::ERR_IO_PENDING) {
    ReadDataComplete(read_data_callback, buffer, result);
  }
}

void GeneratedCodeCache::ReadDataComplete(
    ReadDataCallback callback,
    scoped_refptr<net::IOBufferWithSize> buffer,
    int rv) {
  if (rv != buffer->size()) {
    std::move(callback).Run(scoped_refptr<net::IOBufferWithSize>());
  } else {
    std::move(callback).Run(buffer);
  }
}

void GeneratedCodeCache::DeleteEntryImpl(const std::string& key) {
  if (backend_state_ != kInitialized)
    return;

  backend_->DoomEntry(key, net::LOWEST, net::CompletionOnceCallback());
}

}  // namespace content
