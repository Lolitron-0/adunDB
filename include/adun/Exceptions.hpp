#pragma once

#include <stdexcept>
namespace adun {

class TableException : public std::runtime_error {
protected:
  explicit TableException(const std::string& msg)
      : std::runtime_error{ msg } {
  }
};

} // namespace adun
