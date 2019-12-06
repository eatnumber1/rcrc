#ifndef RCRC_REMOTE_H_
#define RCRC_REMOTE_H_

#include <string_view>
#include <cstdint>
#include <cstdlib>
#include <memory>

#include "rhutil/status.h"
#include "curl/curl.h"
#include "rhutil/curl/curl.h"
#include "nlohmann/json.hpp"
#include "rhutil/json/json.h"

namespace rcrc {

class Remote {
 public:
  struct Options {
    rhutil::CurlURL url =
        rhutil::CurlURL::FromStringOrDie("http://localhost:5572");
    bool verbose = false;
  };

  static rhutil::StatusOr<Remote> Create(Options opts);

  Remote() = default;

  Remote(const Remote &);
  Remote &operator=(const Remote &);
  Remote(Remote &&) = default;
  Remote &operator=(Remote &&) = default;

  Remote UnsharedCopy() const;

  rhutil::Status OperationsList(
      nlohmann::json parameters,
      std::function<rhutil::Status(nlohmann::json)> callback);

  rhutil::Status CoreQuit(int exit_code = EXIT_SUCCESS);

  rhutil::StatusOr<nlohmann::json> RcNoop(nlohmann::json parameters);
  rhutil::StatusOr<nlohmann::json> RcNoopAuth(nlohmann::json parameters);

  rhutil::StatusOr<nlohmann::json> Call(
      std::string_view op, nlohmann::json parameters,
      rhutil::JSONParser::Callback callback = nullptr);

 private:
  Options options() const;

  static rhutil::StatusOr<Remote> Create(
      Options opts, std::shared_ptr<rhutil::ThreadSafeCurlShare> share);

  Remote(Options opts,
         std::shared_ptr<rhutil::ThreadSafeCurlShare> share,
         std::unique_ptr<curl_slist, rhutil::CurlSListDeleter> headers,
         std::unique_ptr<CURL, rhutil::CurlHandleDeleter> handle);

  bool verbose_;
  std::shared_ptr<rhutil::ThreadSafeCurlShare> share_;
  rhutil::CurlURL url_;
  std::unique_ptr<curl_slist, rhutil::CurlSListDeleter> headers_;
  std::unique_ptr<CURL, rhutil::CurlHandleDeleter> handle_;
};

rhutil::Status InitializeGlobals();

}  // namespace rcrc

#endif  // RCRC_REMOTE_H_
