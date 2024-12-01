#include "adun/Exceptions.hpp"
#include <fmt/format.h>

namespace adun {

NoSuchColumnException::NoSuchColumnException(const std::string& name)
    : DatabaseException{ fmt::format("Column {} does not exist", name) } {
}

} // namespace adun
