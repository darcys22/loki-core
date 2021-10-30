#pragma once

#include "decoy.hpp"

#include <functional>

namespace wallet
{
  class Wallet;
  class Block;

  class DaemonComms
  {
   public:
    virtual void
    GetHeight(std::function<void(uint64_t height)> cb) = 0;

    virtual void
    GetBlocks(
        uint64_t start_height, uint64_t end_height, std::function<void(std::vector<Block>)> cb) = 0;

    virtual void
    RegisterWallet(Wallet& wallet) = 0;
  };

}  // namespace wallet
