#include "rcrc/remote.h"

#include <string>
#include <optional>

#include "rhutil/curl/curl.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"

namespace rcrc {

using ::rhutil::CurlCodeToStatus;
using ::rhutil::CurlEasyGetInfo;
using ::rhutil::CurlEasyPerform;
using ::rhutil::CurlEasySetWriteCallback;
using ::rhutil::CurlEasySetopt;
using ::rhutil::CurlHandleDeleter;
using ::rhutil::CurlSListDeleter;
using ::rhutil::CurlShareSetopt;
using ::rhutil::CurlStrDeleter;
using ::rhutil::CurlURL;
using ::rhutil::HTTPCodeToStatus;
using ::rhutil::JSONParser;
using ::rhutil::InternalError;
using ::rhutil::InternalErrorBuilder;
using ::rhutil::InvalidArgumentErrorBuilder;
using ::rhutil::NewCurlSList;
using ::rhutil::OkStatus;
using ::rhutil::Status;
using ::rhutil::StatusBuilder;
using ::rhutil::StatusOr;
using ::rhutil::ThreadSafeCurlShare;
using Options = ::rcrc::Remote::Options;
using json = ::nlohmann::json;
using ParseEvent = ::rhutil::JSONParser::ParseEvent;
using CallbackAction = ::rhutil::JSONParser::CallbackAction;

namespace {

Status HTTPResponseStatus(CURL *handle, Status *ret) {
  long response_code = -1;
  RETURN_IF_ERROR(CurlEasyGetInfo(handle, CURLINFO_RESPONSE_CODE,
                                  &response_code));
  *ret = HTTPCodeToStatus(response_code);
  return OkStatus();
}

}  // namespace

Remote::Remote(Options opts,
               std::shared_ptr<ThreadSafeCurlShare> share,
               std::unique_ptr<curl_slist, CurlSListDeleter> headers,
               std::unique_ptr<CURL, CurlHandleDeleter> handle)
  : verbose_(opts.verbose), share_(std::move(share)), url_(std::move(opts.url)),
    headers_(std::move(headers)), handle_(std::move(handle)) {}

Remote::Remote(const Remote &o) {
  if (o.handle_) {
    *this = Remote::Create(o.options(), o.share_).ValueOrDie();
  } else {
    Remote{std::move(*this)};
  }
}

Remote &Remote::operator=(const Remote &o) {
  return *this = Remote(o);
}

Remote Remote::UnsharedCopy() const {
  CHECK(handle_);
  return Remote::Create(options()).ValueOrDie();
}

Options Remote::options() const {
  Options opts;
  opts.url = url_;
  opts.verbose = verbose_;
  return opts;
}

StatusOr<Remote> Remote::Create(Options opts) {
  auto share = std::make_shared<ThreadSafeCurlShare>();
  RETURN_IF_ERROR(CurlShareSetopt(share->ptr(), CURLSHOPT_SHARE,
                                  CURL_LOCK_DATA_CONNECT));
  return Create(opts, std::move(share));
}

StatusOr<Remote> Remote::Create(
    Options opts, std::shared_ptr<rhutil::ThreadSafeCurlShare> share) {
  auto handle = rhutil::CurlEasyInit();

  RETURN_IF_ERROR(CurlEasySetopt(handle.get(), CURLOPT_CURLU,
                                 opts.url.GetCURLU()));

  std::unique_ptr<curl_slist, CurlSListDeleter> headers =
      NewCurlSList({"Content-Type: application/json"});
  RETURN_IF_ERROR(CurlEasySetopt(handle.get(), CURLOPT_HTTPHEADER,
                                 headers.get()));

  RETURN_IF_ERROR(CurlEasySetopt(handle.get(), CURLOPT_POST, true));
  RETURN_IF_ERROR(CurlEasySetopt(handle.get(), CURLOPT_VERBOSE, opts.verbose));
  RETURN_IF_ERROR(CurlEasySetopt(handle.get(), CURLOPT_SHARE, share->ptr()));

  return Remote(std::move(opts), std::move(share), std::move(headers),
                std::move(handle));
}

Status Remote::OperationsList(json parameters,
                              std::function<Status(json)> callback) {
  return Call("operations/list", std::move(parameters), 
      [callback=std::move(callback)](
          int depth, ParseEvent event, nlohmann::json *parsed)
          -> StatusOr<CallbackAction> {
        if (event != ParseEvent::object_end || depth != 2) {
          return CallbackAction::KEEP;
        }
        RETURN_IF_ERROR(callback(std::move(*parsed)));
        return CallbackAction::DISCARD;
      }).status();
}

Status Remote::CoreQuit(int exit_code) {
  return Call("core/quit", {{"exitCode", exit_code}}).status();
}

StatusOr<json> Remote::RcNoop(json parameters) {
  return Call("rc/noop", std::move(parameters));
}

StatusOr<json> Remote::RcNoopAuth(json parameters) {
  return Call("rc/noopauth", std::move(parameters));
}

StatusOr<json> Remote::Call(std::string_view op, json parameters,
                            JSONParser::Callback callback) {
  CHECK(handle_);
  std::string params = parameters.dump();
  std::string fixed_op(op);
  if (fixed_op[0] != '/') fixed_op = "/" + fixed_op;

  url_.SetPath(fixed_op);

  RETURN_IF_ERROR(CurlEasySetopt(handle_.get(), CURLOPT_POSTFIELDSIZE_LARGE,
                                 params.size()));
  RETURN_IF_ERROR(CurlEasySetopt(handle_.get(), CURLOPT_POSTFIELDS,
                                 params.data()));

  auto parser = std::make_unique<JSONParser>(std::move(callback));
  bool parse_started = false;
  RETURN_IF_ERROR(CurlEasySetWriteCallback(
      handle_.get(),
      [&parser, &parse_started, this](std::string_view data,
                                      size_t*) -> Status {
        if (!parse_started) {
          parse_started = true;

          Status http_status;
          RETURN_IF_ERROR(HTTPResponseStatus(handle_.get(), &http_status));
          if (!http_status.ok()) parser = std::make_unique<JSONParser>();
        }

        return parser->Parse(data);
      }));

  auto err = CurlEasyPerform(handle_.get());
  auto parsed_or = parser->Complete();
  if (!parsed_or.ok()) {
    if (!err.ok()) return err;
    return InternalErrorBuilder()
        << "Failed to parse response JSON of successful HTTP request: "
        << std::move(parsed_or).status();
  }

  auto parsed = std::move(parsed_or).ValueOrDie();
  if (!err.ok()) return StatusBuilder(std::move(err)) << parsed.dump(2);
  return std::move(parsed);
}

Status InitializeGlobals() {
  return rhutil::CurlGlobalInit();
}

}  // namespace rcrc
