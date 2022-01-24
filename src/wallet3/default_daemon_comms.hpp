#pragma once

#include "daemon_comms.hpp"
#include "cryptonote_config.h"

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
    static constexpr int64_t DEFAULT_MAX_RESPONSE_SIZE = 1 * 1024 * 1024; // 1 MiB
    static constexpr int64_t DEFAULT_MAX_SYNC_BLOCKS = 200;

    void
    on_get_blocks_response(std::vector<std::string> response);

    void
    request_top_block_info();

   public:

    DefaultDaemonComms(std::shared_ptr<oxenmq::OxenMQ> omq);

    void
    set_remote(std::string_view address);

    int64_t
    get_height() { return top_block_height; }

    void
    register_wallet(wallet::Wallet& wallet, int64_t height, bool check_sync_height);

    void
    deregister_wallet(Wallet& wallet, std::promise<void>& p);

    std::pair<int64_t, int64_t>
    get_fee_parameters();

    std::future<std::vector<Decoy>>
    fetch_decoys(const std::vector<int64_t>& indexes);

   private:

    void
    for_each_wallet(std::function<void(std::shared_ptr<Wallet>)> func);

    void
    get_blocks();

    void
    got_blocks(int64_t start_height, int64_t end_height);

    void
    start_syncing();

    std::unordered_map<std::shared_ptr<Wallet>, int64_t> wallets;

    std::shared_ptr<oxenmq::OxenMQ> omq;
    oxenmq::address remote;
    oxenmq::ConnectionID conn;
    oxenmq::TimerID status_timer;
    oxenmq::TaggedThreadID sync_thread;

    int64_t max_response_size = DEFAULT_MAX_RESPONSE_SIZE;

    crypto::hash top_block_hash;
    int64_t top_block_height = 0;

    int64_t sync_from_height = 0;
    bool syncing = false;
    int64_t max_sync_blocks = DEFAULT_MAX_SYNC_BLOCKS;

    int64_t fee_per_byte = FEE_PER_BYTE_V13;
    int64_t fee_per_output = FEE_PER_OUTPUT_V18;
  };

}  // namespace wallet
