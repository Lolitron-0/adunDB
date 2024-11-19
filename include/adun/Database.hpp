#pragma once
#include "adun/Parser/CreateCommand.hpp"
#include "adun/Result.hpp"
#include "adun/Table.hpp"
#include <unordered_map>

namespace adun {

class Database {
public:
  Database() = default;

  auto execute(const std::string& query) -> Result;

  friend class ast::CreateCommand;

private:
  std::unordered_map<std::string, Table> m_Tables;
};

} // namespace adun
