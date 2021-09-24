#include "wallet.hpp"

#include "db_schema.hpp"

#include <common/hex.h>

#include <filesystem>

namespace wallet
{

  Wallet::Wallet(std::shared_ptr<Keyring> _keys,
                 std::shared_ptr<TransactionScanner> _txScanner,
                 std::shared_ptr<TransactionConstructor> _txConstructor,
                 std::shared_ptr<DaemonComms> _daemonComms,
                 std::string_view dbFilename,
                 std::string_view dbPassword)
    :
      db(std::filesystem::path(dbFilename), dbPassword)
  {
  }

  void Wallet::AddBlock(const cryptonote::block& block, const uint64_t height)
  {
    SQLite::Transaction db_tx(*db);

    tools::type_to_hex
    db->exec("INSERT INTO blocks VALUES 
  }

}
