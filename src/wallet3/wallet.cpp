#include "wallet.hpp"

#include "db_schema.hpp"
#include "wallet2½.hpp"

#include <sqlitedb/database.hpp>

#include <common/hex.h>
#include <cryptonote_basic/cryptonote_basic.h>

#include <filesystem>

namespace wallet
{
  Wallet::Wallet(
      std::shared_ptr<Keyring> _keys,
      std::shared_ptr<TransactionScanner> _txScanner,
      std::shared_ptr<TransactionConstructor> _txConstructor,
      std::shared_ptr<DaemonComms> _daemonComms,
      std::string_view dbFilename,
      std::string_view dbPassword)
  {
    db = std::make_unique<db::Database>(std::filesystem::path(dbFilename), dbPassword);
  }

  void
  Wallet::AddBlock(
      const cryptonote::block& block,
      const std::vector<cryptonote::transaction>& transactions,
      const crypto::hash& block_hash,
      const uint64_t height)
  {
    SQLite::Transaction db_tx(db->db);

    db->prepared_exec("INSERT INTO blocks VALUES(?)", tools::type_to_hex(block_hash));

    for (const auto& output : txScanner->ScanTransactionReceived(
             block.miner_tx, wallet2½::tx_hash(block.miner_tx), height, block.timestamp))
    {
      // TODO: this
    }

    for (const auto& tx : transactions)
    {
      for (const auto& output :
           txScanner->ScanTransactionReceived(tx, wallet2½::tx_hash(tx), height, block.timestamp))
      {
        // TODO: this
      }
    }
  }

}  // namespace wallet
