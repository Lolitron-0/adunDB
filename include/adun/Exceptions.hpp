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

class ParserException : public DatabaseException {
public:
  using DatabaseException::DatabaseException;
};

class NoSuchColumnException : public DatabaseException {
public:
  explicit NoSuchColumnException(const std::string& name)
      : DatabaseException{ "Column " + name + " does not exist" } {
  }
};

} // namespace adun
