// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_COMMON_AUTOFILL_FEATURES_H_
#define COMPONENTS_AUTOFILL_CORE_COMMON_AUTOFILL_FEATURES_H_

#include "base/feature_list.h"

namespace autofill {
namespace features {

// All features in alphabetical order.
extern const base::Feature kAutofillAddressNormalizer;
extern const base::Feature kAutofillCacheQueryResponses;
extern const base::Feature kAutofillDownstreamUseGooglePayBrandingOniOS;
extern const base::Feature kAutofillDynamicForms;
extern const base::Feature kAutofillEnableAccountWalletStorage;
extern const base::Feature kAutofillEnablePaymentsInteractionsOnAuthError;
extern const base::Feature kAutofillEnforceMinRequiredFieldsForHeuristics;
extern const base::Feature kAutofillEnforceMinRequiredFieldsForQuery;
extern const base::Feature kAutofillEnforceMinRequiredFieldsForUpload;
extern const base::Feature kAutofillManualFallback;
extern const base::Feature kAutofillPrefilledFields;
extern const base::Feature kAutofillResetFullServerCardsOnAuthError;
extern const base::Feature kAutofillRestrictUnownedFieldsToFormlessCheckout;
extern const base::Feature kAutofillSendExperimentIdsInPaymentsRPCs;
extern const base::Feature kAutofillSendOnlyCountryInGetUploadDetails;
extern const base::Feature kAutofillShowAllSuggestionsOnPrefilledForms;
extern const base::Feature kAutofillShowAutocompleteConsoleWarnings;
extern const base::Feature kAutofillShowTypePredictions;
extern const base::Feature kAutofillSkipComparingInferredLabels;
extern const base::Feature kAutofillUpstreamDisallowElo;
extern const base::Feature kAutofillUpstreamDisallowJcb;
extern const base::Feature kAutofillUpstreamUseGooglePayBrandingOnMobile;
extern const base::Feature kAutomaticPasswordGeneration;
extern const base::Feature kSingleClickAutofill;
extern const base::Feature kAutofillPrefilledFields;
extern const base::Feature kAutofillRationalizeRepeatedServerPredictions;
}  // namespace features
}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_COMMON_AUTOFILL_FEATURES_H_
