#include <boost/assert.hpp>
#include <boost/stacktrace.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>

namespace boost {

void assertion_failed_msg(char const* expr, char const* msg,
                          char const* function, char const* /*file*/,
                          long /*line*/) {
  fmt::print(stderr, fmt::fg(fmt::color::red), "Assertion failed: ");
  fmt::print(stderr, "Expression '{}' is false in function '{}': {}.\n",
             expr, function, (msg ? msg : "<...>"));
  fmt::print(stderr, fmt::fg(fmt::color::orange), "Stacktrace:\n");
  std::cerr << boost::stacktrace::stacktrace() << std::endl;
  std::abort();
}

void assertion_failed(char const* expr, char const* function,
                      char const* file, long line) {
  ::boost::assertion_failed_msg(expr, nullptr, function, file, line);
}

} // namespace boost
