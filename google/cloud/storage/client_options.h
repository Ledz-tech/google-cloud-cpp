// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_CLIENT_OPTIONS_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_CLIENT_OPTIONS_H

#include "google/cloud/storage/oauth2/credentials.h"
#include "google/cloud/storage/version.h"
#include "google/cloud/common_options.h"
#include "google/cloud/options.h"
#include <chrono>
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
class ClientOptions;
namespace internal {
std::string JsonEndpoint(ClientOptions const&);
std::string JsonUploadEndpoint(ClientOptions const&);
std::string XmlEndpoint(ClientOptions const&);
std::string IamEndpoint(ClientOptions const&);

Options MakeOptions(ClientOptions);
Options FillWithDefaults(std::shared_ptr<oauth2::Credentials> credentials,
                         Options = {});

/// Configure the IAM endpoint for the GCS client library.
struct GcsRestEndpointOption {
  using Type = std::string;
};

struct GcsIamEndpointOption {
  using Type = std::string;
};

/// Configure oauth2::Credentials for the GCS client library.
struct Oauth2CredentialsOption {
  using Type = std::shared_ptr<oauth2::Credentials>;
};

/// This is only intended for testing against staging or development versions
/// of the service. It should *remain* in the internal:: namespace.
struct TargetApiVersionOption {
  using Type = std::string;
};

struct ProjectIdOption {
  using Type = std::string;
};

struct ConnectionPoolSizeOption {
  using Type = std::size_t;
};

struct DownloadBufferSizeOption {
  using Type = std::size_t;
};

struct UploadBufferSizeOption {
  using Type = std::size_t;
};

struct MaximumSimpleUploadSizeOption {
  using Type = std::size_t;
};

struct EnableCurlSslLockingOption {
  using Type = bool;
};

struct EnableCurlSigpipeHandlerOption {
  using Type = bool;
};

struct MaximumCurlSocketRecvSizeOption {
  using Type = std::size_t;
};

struct MaximumCurlSocketSendSizeOption {
  using Type = std::size_t;
};

struct DownloadStallTimeoutOption {
  using Type = std::chrono::seconds;
};

struct SslRootPathOption {
  using Type = std::string;
};

}  // namespace internal

/**
 * Describes the configuration for low-level connection features.
 *
 * Some applications may want to use a different SSL root of trust for their
 * connections, for example, containerized applications might store the
 * certificate authority certificates in a hard-coded location.
 *
 * This is a separate class, as it is used to configure both the normal
 * connections to GCS and the connections used to obtain OAuth2 access
 * tokens.
 */
class ChannelOptions {
 public:
  std::string ssl_root_path() const { return ssl_root_path_; }

  ChannelOptions& set_ssl_root_path(std::string ssl_root_path) {
    ssl_root_path_ = std::move(ssl_root_path);
    return *this;
  }

 private:
  std::string ssl_root_path_;
};

/**
 * Describes the configuration for a `storage::Client` object.
 *
 * By default, several environment variables are read to configure the client:
 *
 * - `CLOUD_STORAGE_EMULATOR_ENDPOINT`: if set, use this http endpoint to
 *   make all http requests instead of the production GCS service. Also,
 *   if set, the `CreateDefaultClientOptions()` function will use an
 *   `AnonymousCredentials` object instead of loading Application Default
 *   %Credentials.
 * - `CLOUD_STORAGE_ENABLE_CLOG`: if set, enable std::clog as a backend for
 *   `google::cloud::LogSink`.
 * - `CLOUD_STORAGE_ENABLE_TRACING`: if set, this is the list of components that
 *   will have logging enabled, the component this is:
 *   - `http`: trace all http request / responses.
 */
class ClientOptions {
 public:
  explicit ClientOptions(std::shared_ptr<oauth2::Credentials> credentials)
      : ClientOptions(std::move(credentials), {}) {}
  ClientOptions(std::shared_ptr<oauth2::Credentials> credentials,
                ChannelOptions channel_options);

  /**
   * Creates a `ClientOptions` with Google Application Default %Credentials.
   *
   * If Application Default %Credentials could not be loaded, this returns a
   * `Status` with failure details.  If the `CLOUD_STORAGE_EMULATOR_ENDPOINT`
   * environment variable is set, this function instead uses an
   * `AnonymousCredentials` to configure the client.
   */
  static StatusOr<ClientOptions> CreateDefaultClientOptions();
  static StatusOr<ClientOptions> CreateDefaultClientOptions(
      ChannelOptions const& channel_options);

  std::shared_ptr<oauth2::Credentials> credentials() const {
    return opts_.get<internal::Oauth2CredentialsOption>();
  }
  ClientOptions& set_credentials(std::shared_ptr<oauth2::Credentials> c) {
    opts_.set<internal::Oauth2CredentialsOption>(std::move(c));
    return *this;
  }

  std::string const& endpoint() const {
    return opts_.get<internal::GcsRestEndpointOption>();
  }
  ClientOptions& set_endpoint(std::string endpoint) {
    opts_.set<internal::GcsRestEndpointOption>(std::move(endpoint));
    return *this;
  }

  std::string const& iam_endpoint() const {
    return opts_.get<internal::GcsIamEndpointOption>();
  }
  ClientOptions& set_iam_endpoint(std::string endpoint) {
    opts_.set<internal::GcsIamEndpointOption>(std::move(endpoint));
    return *this;
  }

  std::string const& version() const {
    return opts_.get<internal::TargetApiVersionOption>();
  }
  ClientOptions& set_version(std::string version) {
    opts_.set<internal::TargetApiVersionOption>(std::move(version));
    return *this;
  }

  bool enable_http_tracing() const;
  ClientOptions& set_enable_http_tracing(bool enable);

  bool enable_raw_client_tracing() const;
  ClientOptions& set_enable_raw_client_tracing(bool enable);

  std::string const& project_id() const {
    return opts_.get<internal::ProjectIdOption>();
  }
  ClientOptions& set_project_id(std::string v) {
    opts_.set<internal::ProjectIdOption>(std::move(v));
    return *this;
  }

  std::size_t connection_pool_size() const {
    return opts_.get<internal::ConnectionPoolSizeOption>();
  }
  ClientOptions& set_connection_pool_size(std::size_t size) {
    opts_.set<internal::ConnectionPoolSizeOption>(size);
    return *this;
  }

  std::size_t download_buffer_size() const {
    return opts_.get<internal::DownloadBufferSizeOption>();
  }
  ClientOptions& SetDownloadBufferSize(std::size_t size);

  std::size_t upload_buffer_size() const {
    return opts_.get<internal::UploadBufferSizeOption>();
  }
  ClientOptions& SetUploadBufferSize(std::size_t size);

  std::string const& user_agent_prefix() const { return user_agent_prefix_; }
  ClientOptions& add_user_agent_prefix(std::string prefix) {
    opts_.lookup<UserAgentProductsOption>().push_back(prefix);
    if (!user_agent_prefix_.empty()) {
      prefix += '/';
      prefix += user_agent_prefix_;
    }
    user_agent_prefix_ = std::move(prefix);
    return *this;
  }
  /// @deprecated use `add_user_agent_prefix()` instead.
  ClientOptions& add_user_agent_prefx(std::string const& v) {
    return add_user_agent_prefix(v);
  }

  std::size_t maximum_simple_upload_size() const {
    return opts_.get<internal::MaximumSimpleUploadSizeOption>();
  }
  ClientOptions& set_maximum_simple_upload_size(std::size_t v) {
    opts_.set<internal::MaximumSimpleUploadSizeOption>(v);
    return *this;
  }

  /**
   * If true and using OpenSSL 1.0.2 the library configures the OpenSSL
   * callbacks for locking.
   */
  bool enable_ssl_locking_callbacks() const {
    return opts_.get<internal::EnableCurlSslLockingOption>();
  }
  ClientOptions& set_enable_ssl_locking_callbacks(bool v) {
    opts_.set<internal::EnableCurlSslLockingOption>(v);
    return *this;
  }

  bool enable_sigpipe_handler() const {
    return opts_.get<internal::EnableCurlSigpipeHandlerOption>();
  }
  ClientOptions& set_enable_sigpipe_handler(bool v) {
    opts_.set<internal::EnableCurlSigpipeHandlerOption>(v);
    return *this;
  }

  std::size_t maximum_socket_recv_size() const {
    return opts_.get<internal::MaximumCurlSocketRecvSizeOption>();
  }
  ClientOptions& set_maximum_socket_recv_size(std::size_t v) {
    opts_.set<internal::MaximumCurlSocketRecvSizeOption>(v);
    return *this;
  }

  std::size_t maximum_socket_send_size() const {
    return opts_.get<internal::MaximumCurlSocketSendSizeOption>();
  }
  ClientOptions& set_maximum_socket_send_size(std::size_t v) {
    opts_.set<internal::MaximumCurlSocketSendSizeOption>(v);
    return *this;
  }

  ChannelOptions& channel_options() { return channel_options_; }
  ChannelOptions const& channel_options() const { return channel_options_; }

  //@{
  /**
   * Control the maximum amount of time allowed for "stalls" during a download.
   *
   * A download that receives no data is considered "stalled". If the download
   * remains stalled for more than the time set in this option then the download
   * is aborted.
   *
   * The default value is 2 minutes. Can be disabled by setting the value to 0.
   */
  std::chrono::seconds download_stall_timeout() const {
    return opts_.get<internal::DownloadStallTimeoutOption>();
  }
  ClientOptions& set_download_stall_timeout(std::chrono::seconds v) {
    opts_.set<internal::DownloadStallTimeoutOption>(std::move(v));
    return *this;
  }
  //@}

 private:
  friend std::string internal::JsonEndpoint(ClientOptions const&);
  friend std::string internal::JsonUploadEndpoint(ClientOptions const&);
  friend std::string internal::XmlEndpoint(ClientOptions const&);
  friend std::string internal::IamEndpoint(ClientOptions const&);
  friend Options internal::MakeOptions(ClientOptions);

  Options opts_;

  // Used for backwards compatibility. The value here is merged with the values
  // in `opts_` by internal::MakeOptions(ClientOptions const&);
  ChannelOptions channel_options_;

  // Only used for backwards compatibility, the value in `opts_.
  std::string user_agent_prefix_;
};

}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_CLIENT_OPTIONS_H
