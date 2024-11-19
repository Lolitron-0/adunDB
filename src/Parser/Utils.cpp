#include "adun/Parser/Utils.hpp"
#include <cctype>

namespace adun {

auto startsWith(const SourceIt& pos, std::string_view prefix) -> bool {
  return std::equal(prefix.begin(), prefix.end(), pos);
}

void skipSpacesSince(SourceIt& pos) {
  while (*pos == ' ') {
    ++pos;
  }
}

auto consumeIdent(SourceIt& pos) -> std::string_view {
  size_t length{ 0 };
  auto start{ pos };

  if (!std::isalpha(*pos) && *pos != '_') {
    return {};
  }

  while (std::isalnum(*pos) || *pos == '_') {
    ++pos;
    ++length;
  }
  return std::string_view{ start, start + length };
}

} // namespace adun
