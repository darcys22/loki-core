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

namespace wallet
{
  struct address;  // FIXME: placeholder type

  class Wallet
  {
   public:
    Wallet(
        std::shared_ptr<Keyring> _keys,
        std::shared_ptr<TransactionScanner> _txScanner,
        std::shared_ptr<TransactionConstructor> _txConstructor,
        std::shared_ptr<DaemonComms> _daemonComms,
        std::string_view dbFilename,
        std::string_view dbPassword);

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

    // TODO: error types to throw
    PendingTransaction
    CreateTransaction(
        const std::vector<std::pair<address, uint64_t>>& recipients, uint64_t feePerKB);
    void
    SignTransaction(PendingTransaction& tx);
    void
    SubmitTransaction(const PendingTransaction& tx);

   private:
    void
    AddBlock(
        const cryptonote::block& block,
        const std::vector<cryptonote::transaction>& transactions,
        const crypto::hash& block_hash,
        const uint64_t height);

    std::shared_ptr<Keyring> keys;
    std::shared_ptr<TransactionScanner> txScanner;
    std::shared_ptr<TransactionConstructor> txConstructor;

    std::unique_ptr<db::Database> db;
  };

}  // namespace wallet
