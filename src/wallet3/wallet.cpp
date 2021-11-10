#include "wallet.hpp"

#include "db_schema.hpp"
#include "wallet2½.hpp"
#include "block.hpp"
#include "block_tx.hpp"

#include <common/hex.h>
#include <cryptonote_basic/cryptonote_basic.h>

#include <sqlitedb/database.hpp>
#include <oxenmq/oxenmq.h>

#include <filesystem>

namespace wallet
{
  Wallet::Wallet(
      std::shared_ptr<oxenmq::OxenMQ> oxenMQ,
      std::shared_ptr<Keyring> keys,
      std::shared_ptr<TransactionConstructor> txConstructor,
      std::shared_ptr<DaemonComms> daemonComms,
      std::string_view dbFilename,
      std::string_view dbPassword)
      : oxenMQ(oxenMQ)
      , db{std::make_shared<db::Database>(std::filesystem::path(dbFilename), dbPassword)}
      , keys{keys}
      , txScanner{keys, db}
      , txConstructor{txConstructor}
      , daemonComms{daemonComms}
      , sync_timeout_timer(std::make_shared<oxenmq::TimerID>())
  {
    create_schema(db->db);
    last_scanned_height = db->prepared_get<int64_t>("SELECT last_scan_height FROM metadata WHERE id=0;");
    scan_target_height = static_cast<uint64_t>(
        db->prepared_get<int64_t>("SELECT scan_target_height FROM metadata WHERE id=0;"));
  }

  void
  Wallet::init()
  {
    daemonComms->RegisterWallet(*this);
    RequestNextBlocks();
  }

  Wallet::~Wallet()
  {}

  uint64_t
  Wallet::GetBalance()
  {
    return static_cast<uint64_t>(
        db->prepared_get<int64_t>("SELECT balance FROM metadata WHERE id=0;"));
  }

  uint64_t
  Wallet::ScannedHeight()
  {
    return last_scanned_height == -1 ? 0 : last_scanned_height;
  }

  uint64_t
  Wallet::ScanTargetHeight()
  {
    return scan_target_height;
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
    last_scanned_height++;
  }

  void
  Wallet::AddBlocks(const std::vector<Block>& blocks)
  {
    // for now, only syncing one batch of blocks at a time.  So if a batch comes
    // in that doesn't look like it's the batch we last requested, drop it.  This
    // could happen if multiple wallets are using the same daemon comms object.
    if (blocks.front().height != last_scanned_height + 1)
      return;

    oxenMQ->cancel_timer(*sync_timeout_timer);
    for (const auto& block : blocks)
    {
      AddBlock(block);
    }
    RequestNextBlocks();
  }

  void
  Wallet::UpdateTopBlockInfo(uint64_t height, const crypto::hash& hash)
  {
    auto hash_str = tools::type_to_hex(hash);
    db->prepared_exec("UPDATE metadata SET scan_target_height = ?, scan_target_hash = ? WHERE id = 0",
        static_cast<int64_t>(height), hash_str);

    scan_target_height = height;
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

  void
  Wallet::RequestNextBlocks()
  {
    using namespace std::chrono_literals;

    if (last_scanned_height < -1)
      throw std::runtime_error("Wallet last scan height set to < -1, which should be impossible.");

    auto timeout_job = [self=shared_from_this()](){
      self->RequestNextBlocks();
    };

    oxenMQ->cancel_timer(*sync_timeout_timer);
    oxenMQ->add_timer(*sync_timeout_timer, timeout_job, 15s);

    auto start = static_cast<uint64_t>(last_scanned_height + 1);
    auto end = std::min(start + block_batch_size - 1, scan_target_height);

    if (end >= start)
      daemonComms->GetBlocks(start, end);
  }

}  // namespace wallet
