#include "default_daemon_comms.hpp"

#include "wallet.hpp"
#include "block.hpp"
#include "block_tx.hpp"

namespace wallet
{
  void
  DefaultDaemonComms::GetHeight(std::function<void(uint64_t height)> cb)
  {
    // TODO: This
    cb(0);
  }

  void
  DefaultDaemonComms::GetBlocks(
      uint64_t start_height, uint64_t end_height, std::function<void(std::vector<Block>)> cb)
  {
    (void)start_height;
    (void)end_height;

    std::vector<Block> blocks{};
    cb(blocks);
  }

  void
  DefaultDaemonComms::RegisterWallet(wallet::Wallet& wallet)
  {
    wallets.push_back(wallet.shared_from_this());
  }

}  // namespace wallet
