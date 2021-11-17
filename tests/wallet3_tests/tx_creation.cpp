#include <catch2/catch.hpp>

#include <wallet3/wallet.hpp>

TEST_CASE("DB Schema", "[wallet,tx]")
{

  auto keyring = std::make_shared<wallet::Keyring>(spend_priv, spend_pub, view_priv, view_pub);

  auto ctor = std::make_shared<wallet::TransactionConstructor>();

  SECTION("Creates a successful single transactions")
  {
    REQUIRE_NOTHROW(ctor.CreateTransaction());
  }
}
