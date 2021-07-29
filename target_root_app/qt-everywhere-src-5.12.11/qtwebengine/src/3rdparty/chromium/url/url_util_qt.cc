#include "url/url_util_qt.h"

#include "base/command_line.h"
#include "base/no_destructor.h"
#include "base/numerics/safe_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "url/gurl.h"
#include "url/url_canon.h"
#include "url/url_util.h"

namespace url {

namespace {

std::string ToString(const CustomScheme& cs)
{
  std::string serialized;

  serialized += cs.name;
  serialized += ':';

  switch (cs.type) {
  case SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION:
    serialized += 'u';
    serialized += base::NumberToString(cs.default_port);
    break;
  case SCHEME_WITH_HOST_AND_PORT:
    serialized += 'p';
    serialized += base::NumberToString(cs.default_port);
    break;
  case SCHEME_WITH_HOST:
    serialized += 'h';
    break;
  case SCHEME_WITHOUT_AUTHORITY:
    break;
  }

  if (cs.flags & CustomScheme::Secure)
    serialized += 's';
  if (cs.flags & CustomScheme::Local)
    serialized += 'l';
  if (cs.flags & CustomScheme::LocalAccessAllowed)
    serialized += 'L';
  if (cs.flags & CustomScheme::NoAccessAllowed)
    serialized += 'N';
  if (cs.flags & CustomScheme::ServiceWorkersAllowed)
    serialized += 'W';
  if (cs.flags & CustomScheme::ViewSourceAllowed)
    serialized += 'V';
  if (cs.flags & CustomScheme::ContentSecurityPolicyIgnored)
    serialized += 'C';

  return serialized;
}

class Parser {
public:
  void CharacterArrived(char ch) {
    switch (state) {
    case NAME: CharacterArrivedWhileParsingName(ch); break;
    case OPTIONS: CharacterArrivedWhileParsingOptions(ch); break;
    case PORT: CharacterArrivedWhileParsingPort(ch); break;
    }
  }

  void EndReached() {
    if (!default_port_string.empty())
      FlushPort();
    if (!cs.name.empty())
      Flush();
  }

private:
  void CharacterArrivedWhileParsingName(char ch) {
    switch (ch) {
    case ':': state = OPTIONS; break;
    case ';': Flush(); break;
    default: cs.name += ch; break;
    }
  }

  void CharacterArrivedWhileParsingOptions(char ch) {
    switch (ch) {
    case 'u': cs.type = SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION; state = PORT; break;
    case 'p': cs.type = SCHEME_WITH_HOST_AND_PORT; state = PORT; break;
    case 'h': cs.type = SCHEME_WITH_HOST; break;
    case 's': cs.flags |= CustomScheme::Secure; break;
    case 'l': cs.flags |= CustomScheme::Local; break;
    case 'L': cs.flags |= CustomScheme::LocalAccessAllowed; break;
    case 'N': cs.flags |= CustomScheme::NoAccessAllowed; break;
    case 'W': cs.flags |= CustomScheme::ServiceWorkersAllowed; break;
    case 'V': cs.flags |= CustomScheme::ViewSourceAllowed; break;
    case 'C': cs.flags |= CustomScheme::ContentSecurityPolicyIgnored; break;
    case ';': Flush(); state = NAME; break;
    default: CHECK(false) << "Unexpected character '" << ch << "'.";
    }
  }

  void CharacterArrivedWhileParsingPort(char ch) {
    if (base::IsAsciiDigit(ch)) {
      default_port_string += ch;
      return;
    }

    FlushPort();

    state = OPTIONS;
    CharacterArrivedWhileParsingOptions(ch);
  }

  void FlushPort() {
    CHECK(base::StringToInt(default_port_string, &cs.default_port))
      << "Failed to parse '" << default_port_string << "'.";
    default_port_string.clear();
  }

  void Flush() {
    CustomScheme::AddScheme(cs);
    cs = CustomScheme();
  }

  enum { NAME, OPTIONS, PORT } state = NAME;
  CustomScheme cs;
  std::string default_port_string;
};

} // namespace

std::vector<CustomScheme>& CustomScheme::GetMutableSchemes() {
  static base::NoDestructor<std::vector<CustomScheme>> schemes;
  return *schemes;
}

const std::vector<CustomScheme>& CustomScheme::GetSchemes() {
  return GetMutableSchemes();
}

void CustomScheme::ClearSchemes()
{
  GetMutableSchemes().clear();
}

void CustomScheme::AddScheme(const CustomScheme& cs)
{
  DCHECK(!cs.name.empty());
  DCHECK_EQ(cs.has_port_component(), (cs.default_port != PORT_UNSPECIFIED))
    << "Scheme '" << cs.name << "' has invalid configuration.";
  DCHECK_EQ(base::ToLowerASCII(cs.name), cs.name)
    << "Scheme '" << cs.name << "' should be lower-case.";
  DCHECK(!FindScheme(cs.name))
    << "Scheme '" << cs.name << "' already added.";

  GetMutableSchemes().push_back(cs);
}

const CustomScheme* CustomScheme::FindScheme(base::StringPiece name)
{
  for (const CustomScheme& cs : GetSchemes())
    if (base::LowerCaseEqualsASCII(name, cs.name))
      return &cs;
  return nullptr;
}

const char CustomScheme::kCommandLineFlag[] = "webengine-schemes";

void CustomScheme::SaveSchemes(base::CommandLine* command_line)
{
  std::string serialized;

  for (const CustomScheme& cs : GetSchemes()) {
    if (!serialized.empty())
      serialized += ';';
    serialized += ToString(cs);
  }

  command_line->AppendSwitchASCII(kCommandLineFlag, std::move(serialized));
}

void CustomScheme::LoadSchemes(const base::CommandLine* command_line)
{
  std::string serialized = command_line->GetSwitchValueASCII(kCommandLineFlag);
  Parser parser;
  for (char ch : serialized)
    parser.CharacterArrived(ch);
  parser.EndReached();
}

} // namespace url
