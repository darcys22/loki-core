#pragma once

#include "daemon_comms.hpp"

#include <crypto/crypto.h>

#include <oxenmq/oxenmq.h>

#include <list>
#include <memory>

namespace wallet
{
  struct Wallet;
  struct Block;
  struct BlockTX;

  class DefaultDaemonComms : public DaemonComms, public std::enable_shared_from_this<DefaultDaemonComms>
  {
   private:
    void
    OnGetBlocksResponse(std::vector<std::string> response);

    void
    RequestTopBlockInfo();

    void
    UpdateTopBlockInfo();

   public:

    DefaultDaemonComms(std::shared_ptr<oxenmq::OxenMQ> oxenMQ);

    void
    SetRemote(std::string_view address);

    uint64_t
    GetHeight() { return top_block_height; }

    void
    GetBlocks(
        uint64_t start_height, uint64_t end_height);

    void
    RegisterWallet(wallet::Wallet& wallet);

   private:

    void
    ForEachWallet(std::function<void(std::shared_ptr<Wallet>)> func);

    std::list<std::weak_ptr<Wallet>> wallets;

    std::shared_ptr<oxenmq::OxenMQ> oxenMQ;
    oxenmq::address remote;
    oxenmq::ConnectionID conn;
    oxenmq::TimerID status_timer;

    crypto::hash top_block_hash;
    uint64_t top_block_height = 0;
  };

}  // namespace wallet
