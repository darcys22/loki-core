#include "default_daemon_comms.hpp"

#include "wallet.hpp"
#include "wallet2½.hpp"
#include "block.hpp"
#include "block_tx.hpp"

#include <common/string_util.h>
#include <epee/misc_log_ex.h>

#include <iostream>

namespace wallet
{
  void
  DefaultDaemonComms::OnGetBlocksResponse(std::vector<std::string> response)
  {
    std::vector<Block> blocks;

    try
    {
      oxenmq::bt_dict_consumer dc{response[1]};

      if (not dc.skip_until("blocks"))
        return;

      if (not dc.is_list())
        return;

      // for each block dict
      auto blocks_list = dc.consume_list_consumer();
      while (not blocks_list.is_finished())
      {
        if (not blocks_list.is_dict())
          return;

        Block b;
        auto block_dict = blocks_list.consume_dict_consumer();

        if (block_dict.key() != "hash")
          return;
        b.hash = tools::make_from_guts<crypto::hash>(block_dict.consume_string_view());

        if (block_dict.key() != "height")
          return;
        b.height = block_dict.consume_integer<uint64_t>();

        if (block_dict.key() != "timestamp")
          return;
        b.timestamp = block_dict.consume_integer<uint64_t>();

        if (block_dict.key() != "transactions")
          return;
        auto txs_list = block_dict.consume_list_consumer();

        while (not txs_list.is_finished())
        {
          if (not txs_list.is_dict())
            return;

          BlockTX tx;

          auto tx_dict = txs_list.consume_dict_consumer();

          if (tx_dict.key() != "global_indices")
            return;
          tx.global_indices = tx_dict.consume_list<std::vector<uint64_t> >();

          if (tx_dict.key() != "hash")
            return;
          tx.hash = tools::make_from_guts<crypto::hash>(tx_dict.consume_string_view());

          if (tx_dict.key() != "tx")
            return;

          tx.tx = wallet25::tx_from_blob(tx_dict.consume_string_view());

          if (not tx_dict.is_finished())
            return;

          b.transactions.push_back(tx);
        }

        if (not block_dict.is_finished())
          return;

        blocks.push_back(b);
      }
    }
    catch (...)
    {
      //TODO: logging
      return;
    }

    if (not blocks.size())
      //TODO: logging
      return;

    ForEachWallet([&](std::shared_ptr<Wallet> wallet){ wallet->AddBlocks(blocks); });
  }

  void
  DefaultDaemonComms::RequestTopBlockInfo()
  {
    auto timeout_job = [self=weak_from_this()](){
      if (auto comms = self.lock())
        comms->RequestTopBlockInfo();
    };

    oxenMQ->cancel_timer(status_timer);
    if (top_block_height == 0)
    {
      oxenMQ->add_timer(status_timer, timeout_job, 3s);
    }
    else
      oxenMQ->add_timer(status_timer, timeout_job, 15s);

    oxenMQ->request(conn, "rpc.get_height",
        [self = weak_from_this()](bool ok, std::vector<std::string> response)
        {
          if (not ok or response.size() != 2 or response[0] != "200")
            return;

          if (auto shared_self = self.lock())
          {
            oxenmq::bt_dict_consumer dc{response[1]};

            uint64_t new_height = 0;
            crypto::hash new_hash;

            if (not dc.skip_until("hash"))
              throw std::runtime_error("bad response from rpc.get_height, key 'hash' missing");
            new_hash = tools::make_from_guts<crypto::hash>(dc.consume_string_view());

            if (not dc.skip_until("height"))
              throw std::runtime_error("bad response from rpc.get_height, key 'height' missing");
            new_height = dc.consume_integer<uint64_t>();

            shared_self->top_block_hash = new_hash;
            shared_self->top_block_height = new_height - 1;

            shared_self->ForEachWallet([&](std::shared_ptr<Wallet> wallet) { wallet->UpdateTopBlockInfo(new_height - 1, new_hash); });
          }
        }, "de");
  }

  DefaultDaemonComms::DefaultDaemonComms(std::shared_ptr<oxenmq::OxenMQ> oxenMQ)
    : oxenMQ(oxenMQ)
  {
  }

  void
  DefaultDaemonComms::SetRemote(std::string_view address)
  {
    try
    {
      remote = oxenmq::address{address};
    }
    catch (...)
    {
      //TODO: handle this properly
      throw;
    }

    // TODO: proper callbacks
    conn = oxenMQ->connect_remote(remote, [](auto){}, [](auto,auto){});

    RequestTopBlockInfo();
  }

  void
  DefaultDaemonComms::GetBlocks(
      uint64_t start_height, uint64_t end_height)
  {
    auto req_cb = [self = weak_from_this()](bool ok, std::vector<std::string> response)
    {
      if (not ok or response.size() != 2 or response[0] != "200")
      {
        return;
      }

      if (auto shared_self = self.lock())
      {
        shared_self->OnGetBlocksResponse(response);
      }
    };

    std::map<std::string, uint64_t> req_params_dict{{"end_height", end_height}, {"start_height", start_height}};

    oxenMQ->request(conn, "rpc.get_chain_blocks", req_cb, oxenmq::bt_serialize(req_params_dict));
  }

  void
  DefaultDaemonComms::RegisterWallet(wallet::Wallet& wallet)
  {
    wallets.push_back(wallet.shared_from_this());
  }

  void
  DefaultDaemonComms::ForEachWallet(std::function<void(std::shared_ptr<Wallet>)> func)
  {
    for (auto itr = wallets.begin(); itr != wallets.end();)
    {
      if (auto wallet = itr->lock())
      {
        func(wallet);
        itr++;
      }
      else
      {
        wallets.erase(itr);
      }
    }
  }

}  // namespace wallet
