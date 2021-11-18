#include <filesystem>
#include <catch2/catch.hpp>

#include <wallet3/wallet.hpp>
#include <wallet3/db_schema.hpp>
#include <wallet3/default_daemon_comms.hpp>

#include <sqlitedb/database.hpp>

#include "mock_wallet.hpp"


TEST_CASE("Transaction Creation", "[wallet,tx]")
{

  std::vector<wallet::TransactionRecipient> recipients;
  recipients.emplace_back(wallet::address{}, 10);

  auto oxenmq = std::make_shared<oxenmq::OxenMQ>();
  oxenmq->start();

  auto comms = std::make_shared<wallet::DefaultDaemonComms>(oxenmq);
  comms->SetRemote("ipc://./oxend.sock");

  auto wallet = wallet::MockWallet();
  auto ctor = wallet::TransactionConstructor(wallet.GetDB(), comms);
  SECTION("Expect Fail if database is empty")
  {
    REQUIRE_THROWS(ctor.CreateTransaction(recipients, {}));
  }

  wallet.StoreTestTransaction(5);

  SECTION("Creates a successful single transaction")
  {
    std::cout << __FILE__ << ":" << __LINE__ << " (" << __func__ << ") TODO sean remove this - " << "AAAAAAAAAA" << " - debug" << std::endl;
    wallet::PendingTransaction ptx = ctor.CreateTransaction(recipients, {});
    REQUIRE(ptx.recipients.size() == 1);
  }
}
