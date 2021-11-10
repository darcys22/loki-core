#pragma once

#include "decoy.hpp"

namespace wallet
{
  class Wallet;
  class Block;

  class DaemonComms
  {
   public:

    virtual void
    SetRemote(std::string_view address) = 0;

    virtual uint64_t
    GetHeight() = 0;

    virtual void
    GetBlocks(
        uint64_t start_height, uint64_t end_height) = 0;

    virtual void
    RegisterWallet(Wallet& wallet) = 0;
  };

}  // namespace wallet
