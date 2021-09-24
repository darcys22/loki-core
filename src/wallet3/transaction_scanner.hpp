#pragma once

#include "output.hpp"

#include <cryptonote_basic/cryptonote_basic.h>
#include "keyring.hpp"

#include <vector>

namespace wallet
{
  class TransactionScanner
  {
   public:
    TransactionScanner(std::shared_ptr<Keyring> _keys) : wallet_keys(_keys)
    {}

    std::vector<Output>
    ScanTransactionReceived(
        const cryptonote::transaction& tx,
        const crypto::hash& tx_hash,
        uint64_t height,
        uint64_t timestamp);
    std::vector<Output>
    ScanTransactionSpent(
        const cryptonote::transaction& tx,
        const crypto::hash& tx_hash,
        uint64_t height,
        uint64_t timestamp);

   private:
    std::shared_ptr<Keyring> wallet_keys;
  };

}  // namespace wallet
