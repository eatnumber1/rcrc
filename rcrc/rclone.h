#ifndef RCRC_RCLONE_H_
#define RCRC_RCLONE_H_

#include <string_view>
#include <cstdint>
#include <memory>

#include "rhutil/status.h"
#include "curl/curl.h"
#include "rhutil/curl/curl.h"
#include "nlohmann/json.hpp"
#include "rhutil/json/json.h"
#include "rcrc/remote.h"
#include "boost/process.hpp"

namespace rcrc {

class RClone {
 public:
  struct Options {
    Remote::Options remote;
    std::string config_file;
  };

  static rhutil::StatusOr<RClone> Create(Options opts);

  RClone() = default;
  ~RClone();

  RClone(RClone &&) = default;
  RClone &operator=(RClone &&) = default;

  rhutil::Status Shutdown();

  Remote GetRemote();

 private:
  RClone(std::unique_ptr<boost::process::child> child, Remote remote);

  std::unique_ptr<boost::process::child> child_;
  Remote remote_;
};

rhutil::Status InitializeGlobals();

}  // namespace rcrc

#endif  // RCRC_RCLONE_H_
