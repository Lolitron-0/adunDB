#pragma once

#include <stdexcept>
namespace adun {

class DatabaseException : public std::runtime_error {
protected:
  using std::runtime_error::runtime_error;
};

class TableException : public DatabaseException {
protected:
  using DatabaseException::DatabaseException;
};

} // namespace adun
