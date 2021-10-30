#pragma once

#include "daemon_comms.hpp"

#include <list>
#include <memory>

namespace wallet
{
  struct Wallet;
  struct Block;
  struct BlockTX;

  class DefaultDaemonComms : public DaemonComms
  {
   private:
    std::list<std::weak_ptr<Wallet>> wallets;

   public:
    void
    GetHeight(std::function<void(uint64_t height)> cb);

    void
    GetBlocks(
        uint64_t start_height, uint64_t end_height, std::function<void(std::vector<Block>)> cb);

    void
    RegisterWallet(wallet::Wallet& wallet);
  };

}  // namespace wallet
