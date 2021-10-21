#pragma once

#include <cryptonote_basic/cryptonote_basic.h>
#include <crypto/crypto.h>
#include "block_tx.hpp"

namespace wallet
{
  struct Block
  {
    uint64_t height;
    crypto::hash hash;
    uint64_t timestamp;

    std::vector<BlockTX> transactions;
  };

}  // namespace wallet
