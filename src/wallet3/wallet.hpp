#pragma once

#include "transaction_scanner.hpp"
#include "transaction_constructor.hpp"
#include "daemon_comms.hpp"
#include "keyring.hpp"

#include <memory>
#include <string_view>

namespace db
{
  class Database;
}

namespace oxenmq
{
  class OxenMQ;
  class TimerID;
}

namespace wallet
{
  struct Block;

  constexpr size_t DEFAULT_BLOCK_BATCH_SIZE = 200;
  constexpr size_t DEFAULT_SIMULTANEOUS_BATCHES = 1;

  class Wallet : public std::enable_shared_from_this<Wallet>
  {
   private:
    Wallet(
        std::shared_ptr<oxenmq::OxenMQ> oxenMQ,
        std::shared_ptr<Keyring> keys,
        std::shared_ptr<TransactionConstructor> txConstructor,
        std::shared_ptr<DaemonComms> daemonComms,
        std::string_view dbFilename,
        std::string_view dbPassword);

    void
    init();

   public:
    template <typename... T>
    [[nodiscard]] static std::shared_ptr<Wallet>
    MakeWallet(T&&... args)
    {
      std::shared_ptr<Wallet> p{new Wallet(std::forward<T>(args)...)};
      p->init();
      return p;
    }

    ~Wallet();

    uint64_t
    GetBalance();
    uint64_t
    GetUnlockedBalance();
    address
    GetAddress();

    // FIXME: argument nomenclature
    address
    GetSubaddress(uint32_t account, uint32_t index);

    uint64_t
    ScannedHeight();

    uint64_t
    ScanTargetHeight();

    // TODO: error types to throw
    PendingTransaction
    CreateTransaction(
        const std::vector<std::pair<address, uint64_t>>& recipients, uint64_t feePerKB);
    void
    SignTransaction(PendingTransaction& tx);
    void
    SubmitTransaction(const PendingTransaction& tx);

    void
    AddBlock(const Block& block);

    void
    AddBlocks(const std::vector<Block>& blocks);

    // Called by daemon comms to inform of new sync target.
    void
    UpdateTopBlockInfo(uint64_t height, const crypto::hash& hash);

   private:
    void
    StoreTransaction(
        const crypto::hash& tx_hash, const uint64_t height, const std::vector<Output>& outputs);

    void
    StoreSpends(
        const crypto::hash& tx_hash,
        const uint64_t height,
        const std::vector<crypto::key_image>& spends);

    void
    RequestNextBlocks();

    std::shared_ptr<oxenmq::OxenMQ> oxenMQ;

    std::shared_ptr<db::Database> db;

    std::shared_ptr<Keyring> keys;
    TransactionScanner txScanner;
    std::shared_ptr<TransactionConstructor> txConstructor;
    std::shared_ptr<DaemonComms> daemonComms;

    size_t block_batch_size = DEFAULT_BLOCK_BATCH_SIZE;
    uint64_t scan_target_height = 0;
    int64_t last_scanned_height = -1;

    std::shared_ptr<oxenmq::TimerID> sync_timeout_timer;
  };

}  // namespace wallet
