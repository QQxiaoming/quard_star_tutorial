// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/previews/content/previews_optimization_guide.h"

#include "base/bind.h"
#include "base/metrics/histogram_macros.h"
#include "base/task_runner_util.h"
#include "base/task_scheduler/post_task.h"
#include "components/optimization_guide/proto/hints.pb.h"
#include "components/previews/content/previews_hints.h"
#include "components/previews/core/previews_user_data.h"
#include "net/url_request/url_request.h"
#include "url/gurl.h"

namespace previews {

PreviewsOptimizationGuide::PreviewsOptimizationGuide(
    optimization_guide::OptimizationGuideService* optimization_guide_service,
    const scoped_refptr<base::SingleThreadTaskRunner>& io_task_runner)
    : optimization_guide_service_(optimization_guide_service),
      io_task_runner_(io_task_runner),
      background_task_runner_(base::CreateSequencedTaskRunnerWithTraits(
          {base::MayBlock(), base::TaskPriority::BACKGROUND})),
      io_weak_ptr_factory_(this) {
  DCHECK(optimization_guide_service_);
  optimization_guide_service_->AddObserver(this);
}

PreviewsOptimizationGuide::~PreviewsOptimizationGuide() {
  optimization_guide_service_->RemoveObserver(this);
}

bool PreviewsOptimizationGuide::IsWhitelisted(const net::URLRequest& request,
                                              PreviewsType type) const {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  if (!hints_)
    return false;

  int inflation_percent = 0;
  if (!hints_->IsWhitelisted(request.url(), type, &inflation_percent))
    return false;

  previews::PreviewsUserData* previews_user_data =
      previews::PreviewsUserData::GetData(request);
  if (inflation_percent != 0 && previews_user_data)
    previews_user_data->SetDataSavingsInflationPercent(inflation_percent);

  return true;
}

bool PreviewsOptimizationGuide::IsHostWhitelistedAtNavigation(
    const GURL& url,
    previews::PreviewsType type) const {
  DCHECK(io_task_runner_->BelongsToCurrentThread());

  if (!hints_)
    return false;
  return hints_->IsHostWhitelistedAtNavigation(url, type);
}

void PreviewsOptimizationGuide::OnHintsProcessed(
    const optimization_guide::proto::Configuration& config,
    const optimization_guide::ComponentInfo& info) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());

  base::PostTaskAndReplyWithResult(
      background_task_runner_.get(), FROM_HERE,
      base::BindOnce(&PreviewsHints::CreateFromConfig, config, info),
      base::BindOnce(&PreviewsOptimizationGuide::UpdateHints,
                     io_weak_ptr_factory_.GetWeakPtr()));
}

void PreviewsOptimizationGuide::UpdateHints(
    std::unique_ptr<PreviewsHints> hints) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  hints_ = std::move(hints);
}

}  // namespace previews
