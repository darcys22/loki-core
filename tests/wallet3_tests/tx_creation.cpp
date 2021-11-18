#include <filesystem>
#include <catch2/catch.hpp>

#include <wallet3/wallet.hpp>
#include <wallet3/db_schema.hpp>
#include <wallet3/default_daemon_comms.hpp>

#include <sqlitedb/database.hpp>


TEST_CASE("Transaction Creation", "[wallet,tx]")
{

  std::vector<wallet::TransactionRecipient> recipients;
  recipients.emplace_back(wallet::address{},10);

  auto oxenmq = std::make_shared<oxenmq::OxenMQ>();
  oxenmq->start();

  auto comms = std::make_shared<wallet::DefaultDaemonComms>(oxenmq);
  comms->SetRemote("ipc://./oxend.sock");

  auto db = std::make_shared<db::Database>(std::filesystem::path(":memory:"), "");
  wallet::create_schema(db->db);

  auto ctor = wallet::TransactionConstructor(db, comms);
  SECTION("Expect Fail if database is empty")
  {
    REQUIRE_THROWS(ctor.CreateTransaction(recipients, 10));
  }

  SECTION("Creates a successful single transaction")
  {
    wallet::PendingTransaction ptx = ctor.CreateTransaction(recipients, 10);
    REQUIRE(ptx.recipients.size() == 1);
  }
}
