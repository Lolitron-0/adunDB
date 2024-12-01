#pragma once
#include "adun/Parser/CreateCommand.hpp"
#include "adun/Parser/DeleteCommand.hpp"
#include "adun/Parser/InsertCommand.hpp"
#include "adun/Parser/SelectCommand.hpp"
#include "adun/Parser/UpdateCommand.hpp"
#include "adun/Result.hpp"
#include "adun/Table.hpp"
#include <unordered_map>

namespace adun {

class Database {
public:
  Database() = default;

  auto execute(const std::string& query) -> Result;

  friend class ast::CreateCommand;
  friend class ast::InsertCommand;
  friend class ast::SelectCommand;
  friend class ast::UpdateCommand;
  friend class ast::DeleteCommand;

private:
  std::unordered_map<std::string, Table> m_Tables;
};

} // namespace adun
