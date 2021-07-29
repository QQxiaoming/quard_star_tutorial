// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/identity/public/cpp/identity_manager.h"

#include "google_apis/gaia/gaia_auth_util.h"

namespace identity {

namespace {

// Local copy of the account ID used for supervised users (defined in //chrome
// as supervised_users::kSupervisedUserPseudoEmail). Simply copied to avoid
// plumbing it from //chrome all the way down through the Identity Service just
// to handle the corner cases below.
// TODO(860492): Remove this once supervised user support is removed.
const char kSupervisedUserPseudoEmail[] = "managed_user@localhost";

// A made-up Gaia ID to populate the supervised user's AccountInfo with in order
// to maintain the invariant that the AccountInfos passed out by IdentityManager
// always have an account ID, Gaia ID, and email set.
// TODO(860492): Remove this once supervised user support is removed.
const char kSupervisedUserPseudoGaiaID[] = "managed_user_gaia_id";

}  // namespace

IdentityManager::IdentityManager(
    SigninManagerBase* signin_manager,
    ProfileOAuth2TokenService* token_service,
    AccountTrackerService* account_tracker_service,
    GaiaCookieManagerService* gaia_cookie_manager_service)
    : signin_manager_(signin_manager),
      token_service_(token_service),
      account_tracker_service_(account_tracker_service),
      gaia_cookie_manager_service_(gaia_cookie_manager_service) {
  // Initialize the state of the primary account.
  primary_account_info_ = signin_manager_->GetAuthenticatedAccountInfo();

  // Initialize the state of accounts with refresh tokens.
  // |account_id| is moved into |accounts_with_refresh_tokens_|.
  // Do not change this to "const std::string&".
  for (std::string account_id : token_service->GetAccounts()) {
    AccountInfo account_info =
        account_tracker_service_->GetAccountInfo(account_id);

    // In the context of supervised users, the ProfileOAuth2TokenService is used
    // without the AccountTrackerService being used. This is the only case in
    // which the AccountTrackerService will potentially not know about the
    // account. In this context, |account_id| is always set to
    // kSupervisedUserPseudoEmail.
    // TODO(860492): Remove this special case once supervised user support is
    // removed.
    DCHECK(!account_info.IsEmpty() || account_id == kSupervisedUserPseudoEmail);

    if (account_id == kSupervisedUserPseudoEmail && account_info.IsEmpty()) {
      // Populate the information manually to maintain the invariant that the
      // account ID, gaia ID, and email are always set.
      account_info.account_id = account_id;
      account_info.email = kSupervisedUserPseudoEmail;
      account_info.gaia = kSupervisedUserPseudoGaiaID;
    }

    accounts_with_refresh_tokens_.emplace(std::move(account_id),
                                          std::move(account_info));
  }

  signin_manager_->AddObserver(this);
#if !defined(OS_CHROMEOS)
  SigninManager::FromSigninManagerBase(signin_manager_)
      ->set_diagnostics_client(this);
#endif
  token_service_->AddDiagnosticsObserver(this);
  token_service_->AddObserver(this);
  token_service_->set_diagnostics_client(this);
}

IdentityManager::~IdentityManager() {
  signin_manager_->RemoveObserver(this);
#if !defined(OS_CHROMEOS)
  SigninManager::FromSigninManagerBase(signin_manager_)
      ->set_diagnostics_client(nullptr);
#endif
  token_service_->RemoveObserver(this);
  token_service_->RemoveDiagnosticsObserver(this);
  token_service_->set_diagnostics_client(nullptr);
}

AccountInfo IdentityManager::GetPrimaryAccountInfo() const {
#if defined(OS_CHROMEOS)
  // On ChromeOS in production, the authenticated account is set very early in
  // startup and never changed. Hence, the information held by the
  // IdentityManager should always correspond to that held by SigninManager.
  // NOTE: the above invariant is not guaranteed to hold in tests. If you
  // are seeing this DCHECK go off in a testing context, it means that you need
  // to set the IdentityManager's primary account info in the test at the place
  // where you are setting the authenticated account info in the SigninManager.
  // TODO(blundell): Add the API to do this once we hit the first case and
  // document the API to use here.
  DCHECK_EQ(signin_manager_->GetAuthenticatedAccountId(),
            primary_account_info_.account_id);

  // Note: If the primary account's refresh token gets revoked, then the account
  // gets removed from AccountTrackerService (via
  // AccountFetcherService::OnRefreshTokenRevoked), and so SigninManager's
  // GetAuthenticatedAccountInfo is empty (even though
  // GetAuthenticatedAccountId is NOT empty).
  if (!signin_manager_->GetAuthenticatedAccountInfo().account_id.empty()) {
    DCHECK_EQ(signin_manager_->GetAuthenticatedAccountInfo().account_id,
              primary_account_info_.account_id);
    DCHECK_EQ(signin_manager_->GetAuthenticatedAccountInfo().gaia,
              primary_account_info_.gaia);

    // TODO(842670): As described in the bug, AccountTrackerService's email
    // address can be updated after it is initially set on ChromeOS. Figure out
    // right long-term solution for this problem.
    if (signin_manager_->GetAuthenticatedAccountInfo().email !=
        primary_account_info_.email) {
      // This update should only be to move it from normalized form to the form
      // in which the user entered the email when creating the account. The
      // below check verifies that the normalized forms of the two email
      // addresses are identical.
      DCHECK(gaia::AreEmailsSame(
          signin_manager_->GetAuthenticatedAccountInfo().email,
          primary_account_info_.email));
      primary_account_info_.email =
          signin_manager_->GetAuthenticatedAccountInfo().email;
    }
  }
#endif  // defined(OS_CHROMEOS)
  return primary_account_info_;
}

bool IdentityManager::HasPrimaryAccount() const {
  return !primary_account_info_.account_id.empty();
}

std::vector<AccountInfo> IdentityManager::GetAccountsWithRefreshTokens() const {
  // TODO(blundell): It seems wasteful to construct this vector every time this
  // method is called, but it also seems bad to maintain the vector as an ivar
  // along the map.
  std::vector<AccountInfo> accounts;
  accounts.reserve(accounts_with_refresh_tokens_.size());

  for (const auto& pair : accounts_with_refresh_tokens_) {
    accounts.push_back(pair.second);
  }

  return accounts;
}

bool IdentityManager::HasAccountWithRefreshToken(
    const std::string& account_id) const {
  return base::ContainsKey(accounts_with_refresh_tokens_, account_id);
}

bool IdentityManager::HasPrimaryAccountWithRefreshToken() const {
  return HasAccountWithRefreshToken(GetPrimaryAccountInfo().account_id);
}

std::unique_ptr<AccessTokenFetcher>
IdentityManager::CreateAccessTokenFetcherForAccount(
    const std::string& account_id,
    const std::string& oauth_consumer_name,
    const OAuth2TokenService::ScopeSet& scopes,
    AccessTokenFetcher::TokenCallback callback) {
  return std::make_unique<AccessTokenFetcher>(account_id, oauth_consumer_name,
                                              token_service_, scopes,
                                              std::move(callback));
}

void IdentityManager::RemoveAccessTokenFromCache(
    const std::string& account_id,
    const OAuth2TokenService::ScopeSet& scopes,
    const std::string& access_token) {
  // TODO(843510): Consider making the request to ProfileOAuth2TokenService
  // asynchronously once there are no direct clients of PO2TS. This change would
  // need to be made together with changing all callsites to
  // ProfileOAuth2TokenService::RequestAccessToken() to be made asynchronously
  // as well (to maintain ordering in the case where a client removes an access
  // token from the cache and then immediately requests an access token).
  token_service_->InvalidateAccessToken(account_id, scopes, access_token);
}

void IdentityManager::AddObserver(Observer* observer) {
  observer_list_.AddObserver(observer);
}

void IdentityManager::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);
}

void IdentityManager::AddDiagnosticsObserver(DiagnosticsObserver* observer) {
  diagnostics_observer_list_.AddObserver(observer);
}

void IdentityManager::RemoveDiagnosticsObserver(DiagnosticsObserver* observer) {
  diagnostics_observer_list_.RemoveObserver(observer);
}

void IdentityManager::SetPrimaryAccountSynchronouslyForTests(
    const std::string& gaia_id,
    const std::string& email_address,
    const std::string& refresh_token) {
  DCHECK(!refresh_token.empty());
  SetPrimaryAccountSynchronously(gaia_id, email_address, refresh_token);
}

void IdentityManager::SetPrimaryAccountSynchronously(
    const std::string& gaia_id,
    const std::string& email_address,
    const std::string& refresh_token) {
  signin_manager_->SetAuthenticatedAccountInfo(gaia_id, email_address);
  primary_account_info_ = signin_manager_->GetAuthenticatedAccountInfo();

  if (!refresh_token.empty()) {
    token_service_->UpdateCredentials(primary_account_info_.account_id,
                                      refresh_token);
  }
}

#if !defined(OS_CHROMEOS)
void IdentityManager::WillFireGoogleSigninSucceeded(
    const AccountInfo& account_info) {
  // TODO(843510): Consider setting this info and notifying observers
  // asynchronously in response to GoogleSigninSucceeded() once there are no
  // direct clients of SigninManager.
  primary_account_info_ = account_info;
}

void IdentityManager::WillFireGoogleSignedOut(const AccountInfo& account_info) {
  // TODO(843510): Consider setting this info and notifying observers
  // asynchronously in response to GoogleSigninSucceeded() once there are no
  // direct clients of SigninManager.
  DCHECK(account_info.account_id == primary_account_info_.account_id);
  DCHECK(account_info.gaia == primary_account_info_.gaia);
  DCHECK(account_info.email == primary_account_info_.email);
  primary_account_info_ = AccountInfo();
}
#endif

void IdentityManager::GoogleSigninSucceeded(const AccountInfo& account_info) {
  DCHECK(account_info.account_id == primary_account_info_.account_id);
  DCHECK(account_info.gaia == primary_account_info_.gaia);
  DCHECK(account_info.email == primary_account_info_.email);
  for (auto& observer : observer_list_) {
    observer.OnPrimaryAccountSet(account_info);
  }
}

void IdentityManager::GoogleSignedOut(const AccountInfo& account_info) {
  DCHECK(!HasPrimaryAccount());
  for (auto& observer : observer_list_) {
    observer.OnPrimaryAccountCleared(account_info);
  }
}

void IdentityManager::WillFireOnRefreshTokenAvailable(
    const std::string& account_id,
    bool is_valid) {
  DCHECK(!pending_token_available_state_);

  AccountInfo account_info =
      account_tracker_service_->GetAccountInfo(account_id);

  // In the context of supervised users, the ProfileOAuth2TokenService is used
  // without the AccountTrackerService being used. This is the only case in
  // which the AccountTrackerService will potentially not know about the
  // account. In this context, |account_id| is always set to
  // kSupervisedUserPseudoEmail.
  // TODO(860492): Remove this special case once supervised user support is
  // removed.
  DCHECK(!account_info.IsEmpty() || account_id == kSupervisedUserPseudoEmail);
  if (account_id == kSupervisedUserPseudoEmail && account_info.IsEmpty()) {
    // Populate the information manually to maintain the invariant that the
    // account ID, gaia ID, and email are always set.
    account_info.account_id = account_id;
    account_info.email = kSupervisedUserPseudoEmail;
    account_info.gaia = kSupervisedUserPseudoGaiaID;
  }

  // The account might have already been  present (e.g., this method can fire on
  // updating an invalid token to a valid one or vice versa); in this case we
  // sanity-check that the cached account info has the expected values.
  auto iterator = accounts_with_refresh_tokens_.find(account_id);
  if (iterator != accounts_with_refresh_tokens_.end()) {
    DCHECK_EQ(account_info.gaia, iterator->second.gaia);
    DCHECK(gaia::AreEmailsSame(account_info.email, iterator->second.email));
  } else {
    auto insertion_result = accounts_with_refresh_tokens_.emplace(
        account_id, std::move(account_info));
    DCHECK(insertion_result.second);
    iterator = insertion_result.first;
  }

  PendingTokenAvailableState pending_token_available_state;
  pending_token_available_state.account_info = iterator->second;
  pending_token_available_state.refresh_token_is_valid = is_valid;
  pending_token_available_state_ = std::move(pending_token_available_state);
}

void IdentityManager::WillFireOnRefreshTokenRevoked(
    const std::string& account_id) {
  DCHECK(!pending_token_revoked_info_);

  auto iterator = accounts_with_refresh_tokens_.find(account_id);
  if (iterator == accounts_with_refresh_tokens_.end()) {
    // A corner case exists wherein the token service revokes tokens while
    // loading tokens during initial startup. This is the only case in which it
    // is expected that we could receive this notification without having
    // previously received a notification that this account was available. In
    // this case, we simply do not forward on the notification, for the
    // following reasons: (1) We may not have a fully-populated |account_info|
    // to send as the argument. (2) Sending the notification breaks clients'
    // expectations that IdentityManager will only fire RefreshTokenRemoved
    // notifications for accounts that it previously knew about.
    DCHECK(!token_service_->AreAllCredentialsLoaded());
    return;
  }

  accounts_with_refresh_tokens_.erase(iterator);

  pending_token_revoked_info_ =
      account_tracker_service_->GetAccountInfo(account_id);

  // In the context of supervised users, the ProfileOAuth2TokenService is used
  // without the AccountTrackerService being used. This is the only case in
  // which the AccountTrackerService will potentially not know about the
  // account. In this context, |account_id| is always set to
  // kSupervisedUserPseudoEmail.

  // TODO(860492): Remove this special case once supervised user support is
  // removed.
  DCHECK(!pending_token_revoked_info_->IsEmpty() ||
         account_id == kSupervisedUserPseudoEmail);
  if (account_id == kSupervisedUserPseudoEmail &&
      pending_token_revoked_info_->IsEmpty()) {
    // Populate the information manually to maintain the invariant that the
    // account ID, gaia ID, and email are always set.
    pending_token_revoked_info_->account_id = account_id;
    pending_token_revoked_info_->email = account_id;
    pending_token_revoked_info_->gaia = kSupervisedUserPseudoGaiaID;
  }
}

void IdentityManager::OnRefreshTokenAvailable(const std::string& account_id) {
  DCHECK(pending_token_available_state_);
  DCHECK_EQ(pending_token_available_state_->account_info.account_id,
            account_id);

  // Move the state out of |pending_token_available_state_| in case any observer
  // callbacks fired below result in mutations of refresh tokens.
  AccountInfo account_info =
      std::move(pending_token_available_state_->account_info);
  bool refresh_token_is_valid =
      pending_token_available_state_->refresh_token_is_valid;

  pending_token_available_state_.reset();

  for (auto& observer : observer_list_) {
    observer.OnRefreshTokenUpdatedForAccount(account_info,
                                             refresh_token_is_valid);
  }
}

void IdentityManager::OnRefreshTokenRevoked(const std::string& account_id) {
  // NOTE: It is possible for |pending_token_revoked_info_| to be null in the
  // corner case of tokens being revoked during initial startup (see
  // WillFireOnRefreshTokenRevoked() above).
  if (!pending_token_revoked_info_) {
    return;
  }

  DCHECK_EQ(pending_token_revoked_info_->account_id, account_id);

  // Move the state out of |pending_token_revoked_info_| in case any observer
  // callbacks fired below result in mutations of refresh tokens.
  AccountInfo account_info = pending_token_revoked_info_.value();
  pending_token_revoked_info_.reset();

  for (auto& observer : observer_list_) {
    observer.OnRefreshTokenRemovedForAccount(account_info);
  }
}

void IdentityManager::OnAccessTokenRequested(
    const std::string& account_id,
    const std::string& consumer_id,
    const OAuth2TokenService::ScopeSet& scopes) {
  // TODO(843510): Consider notifying observers asynchronously once there
  // are no direct clients of ProfileOAuth2TokenService.
  for (auto& observer : diagnostics_observer_list_) {
    observer.OnAccessTokenRequested(account_id, consumer_id, scopes);
  }
}

}  // namespace identity
