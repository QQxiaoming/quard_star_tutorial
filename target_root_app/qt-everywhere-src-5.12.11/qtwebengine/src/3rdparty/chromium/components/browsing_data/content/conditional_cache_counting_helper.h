// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSING_DATA_CONTENT_CONDITIONAL_CACHE_COUNTING_HELPER_H_
#define COMPONENTS_BROWSING_DATA_CONTENT_CONDITIONAL_CACHE_COUNTING_HELPER_H_

#include <memory>

#include "base/callback_forward.h"
#include "base/sequenced_task_runner_helpers.h"
#include "net/base/net_errors.h"
#include "net/disk_cache/disk_cache.h"

namespace content {
class StoragePartition;
}

namespace net {
class URLRequestContextGetter;
}

namespace browsing_data {

// Helper to count the size of the http cache data from a StoragePartition.
class ConditionalCacheCountingHelper {
 public:
  // Returns if this value is an upper estimate and the number bytes in the
  // selected range.
  typedef base::OnceCallback<void(bool, int64_t)> CacheCountCallback;

  // Counts the cache entries according to the specified time range.
  // Must be called on the UI thread.
  //
  // The |completion_callback| will be invoked when the operation completes.
  static void Count(content::StoragePartition* storage_partition,
                    base::Time begin_time,
                    base::Time end_time,
                    CacheCountCallback result_callback);

 private:
  enum class CacheState {
    NONE,
    CREATE_MAIN,
    CREATE_MEDIA,
    COUNT_MAIN,
    COUNT_MEDIA,
    DONE
  };

  friend class base::DeleteHelper<ConditionalCacheCountingHelper>;

  ConditionalCacheCountingHelper(
      base::Time begin_time,
      base::Time end_time,
      net::URLRequestContextGetter* main_context_getter,
      net::URLRequestContextGetter* media_context_getter,
      CacheCountCallback result_callback);
  ~ConditionalCacheCountingHelper();

  void Finished();

  void CountHttpCacheOnIOThread();
  void DoCountCache(int rv);

  // State used for legacy path. Will eventually go away.

  // Stores the cache size computation result before it can be returned
  // via a callback. This is either the sum of size of the the two cache
  // backends, or an error code if the calculation failed.
  int64_t calculation_result_;
  bool is_upper_limit_;

  CacheCountCallback result_callback_;
  const base::Time begin_time_;
  const base::Time end_time_;

  bool is_finished_;

  const scoped_refptr<net::URLRequestContextGetter> main_context_getter_;
  const scoped_refptr<net::URLRequestContextGetter> media_context_getter_;

  CacheState next_cache_state_;
  disk_cache::Backend* cache_;

  std::unique_ptr<disk_cache::Backend::Iterator> iterator_;

  DISALLOW_COPY_AND_ASSIGN(ConditionalCacheCountingHelper);
};

}  // namespace browsing_data

#endif  // COMPONENTS_BROWSING_DATA_CONTENT_CONDITIONAL_CACHE_COUNTING_HELPER_H_
