#include "wallet.hpp"

#include "db_schema.hpp"
#include "wallet2½.hpp"
#include "block.hpp"
#include "block_tx.hpp"

#include <sqlitedb/database.hpp>

#include <common/hex.h>
#include <cryptonote_basic/cryptonote_basic.h>

#include <filesystem>

namespace wallet
{
  Wallet::Wallet(
      std::shared_ptr<Keyring> _keys,
      std::shared_ptr<TransactionConstructor> _txConstructor,
      std::shared_ptr<DaemonComms> _daemonComms,
      std::string_view dbFilename,
      std::string_view dbPassword)
      : db{std::make_shared<db::Database>(std::filesystem::path(dbFilename), dbPassword)}
      , keys{_keys}
      , txScanner{keys, db}
      , txConstructor{_txConstructor}
      , daemonComms{_daemonComms}
  {
    create_schema(db->db);
  }

  void
  Wallet::init()
  {}

  Wallet::~Wallet()
  {}

  uint64_t
  Wallet::GetBalance()
  {
    return static_cast<uint64_t>(
        db->prepared_get<int64_t>("SELECT balance FROM metadata WHERE id=0;"));
  }

  void
  Wallet::AddBlock(const Block& block)
  {
    SQLite::Transaction db_tx(db->db);

    db->prepared_exec(
        "INSERT INTO blocks(height,hash,timestamp) VALUES(?,?,?)",
        static_cast<int64_t>(block.height),
        tools::type_to_hex(block.hash),
        static_cast<int64_t>(block.timestamp));

    for (const auto& tx : block.transactions)
    {
      if (auto outputs = txScanner.ScanTransactionReceived(tx, block.height, block.timestamp);
          not outputs.empty())
      {
        StoreTransaction(tx.hash, block.height, outputs);
      }

      if (auto spends = txScanner.ScanTransactionSpent(tx.tx); not spends.empty())
      {
        StoreSpends(tx.hash, block.height, spends);
      }
    }

    db_tx.commit();
  }

  void
  Wallet::StoreTransaction(
      const crypto::hash& tx_hash, const uint64_t height, const std::vector<Output>& outputs)
  {
    auto hash_str = tools::type_to_hex(tx_hash);
    db->prepared_exec(
        "INSERT INTO transactions(block,hash) VALUES(?,?)", static_cast<int64_t>(height), hash_str);

    for (const auto& output : outputs)
    {
      db->prepared_exec(
          "INSERT INTO key_images(key_image) VALUES(?)", tools::type_to_hex(output.key_image));
      db->prepared_exec(
          R"(
          INSERT INTO outputs(
            amount,
            output_index,
            global_index,
            unlock_time,
            block_height,
            tx,
            output_key,
            rct_mask,
            key_image,
            subaddress_major,
            subaddress_minor)
          VALUES(?,?,?,?,?,
            (SELECT id FROM transactions WHERE hash = ?),
            ?,?,
            (SELECT id FROM key_images WHERE key_image = ?),
            ?,?);
          )",
          static_cast<int64_t>(output.amount),
          static_cast<int64_t>(output.output_index),
          static_cast<int64_t>(output.global_index),
          static_cast<int64_t>(output.unlock_time),
          static_cast<int64_t>(output.block_height),
          hash_str,
          tools::type_to_hex(output.key),
          tools::type_to_hex(output.rct_mask),
          tools::type_to_hex(output.key_image),
          output.subaddress_index.major,
          output.subaddress_index.minor);
    }
  }

  void
  Wallet::StoreSpends(
      const crypto::hash& tx_hash,
      const uint64_t height,
      const std::vector<crypto::key_image>& spends)
  {
    auto hash_hex = tools::type_to_hex(tx_hash);
    db->prepared_exec(
        "INSERT INTO transactions(block,hash) VALUES(?,?) ON CONFLICT DO NOTHING",
        static_cast<int64_t>(height),
        hash_hex);

    for (const auto& key_image : spends)
    {
      db->prepared_exec(
          R"(INSERT INTO spends(key_image, height, tx)
          VALUES((SELECT id FROM key_images WHERE key_image = ?),
          ?,
          (SELECT id FROM transactions WHERE hash = ?));)",
          tools::type_to_hex(key_image),
          static_cast<int64_t>(height),
          hash_hex);
    }
  }

}  // namespace wallet
