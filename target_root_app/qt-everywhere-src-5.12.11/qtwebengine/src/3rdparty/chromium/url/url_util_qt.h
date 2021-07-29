#ifndef URL_URL_UTIL_QT_H_
#define URL_URL_UTIL_QT_H_

#include <string>

#include "url/url_util.h"

namespace base {
class CommandLine;
} // namespace base

namespace url {

// Configuration of a custom scheme.
//
// Each process has a list of CustomSchemes. The list is filled in the main
// process and transmitted to subprocesses via command-line flags
// (SaveSchemes/LoadSchemes). We cannot use IPC for this because the url library
// scheme lists are filled and locked before IPC is initialized.
//
// To implement the required semantics, the lists are accessed not only from the
// url library but all over the codebase (grep CustomScheme).
struct URL_EXPORT CustomScheme {
  enum Flag {
    Secure = 0x1,
    Local = 0x2,
    LocalAccessAllowed = 0x4,
    NoAccessAllowed = 0x8,
    ServiceWorkersAllowed = 0x10,
    ViewSourceAllowed = 0x20,
    ContentSecurityPolicyIgnored = 0x40,
  };

  std::string name;
  SchemeType type = SCHEME_WITHOUT_AUTHORITY;
  int default_port = PORT_UNSPECIFIED;
  int flags = 0;

  bool has_host_component() const { return type != SCHEME_WITHOUT_AUTHORITY; }
  bool has_port_component() const { return type <= SCHEME_WITH_HOST_AND_PORT; }

  static const std::vector<CustomScheme>& GetSchemes();
  static std::vector<CustomScheme>& GetMutableSchemes();
  static void ClearSchemes();

  static void AddScheme(const CustomScheme& cs);
  static const CustomScheme* FindScheme(base::StringPiece name);

  static const char kCommandLineFlag[];
  static void SaveSchemes(base::CommandLine* command_line);
  static void LoadSchemes(const base::CommandLine* command_line);
};

} // namespace url

#endif // URL_URL_UTIL_QT_H_
