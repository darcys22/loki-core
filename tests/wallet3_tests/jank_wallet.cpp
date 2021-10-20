#include <cryptonote_core/cryptonote_core.h>
#include <wallet3/wallet.hpp>
#include <wallet3/keyring.hpp>
#include <common/hex.h>

int main(void)
{

  cryptonote::BlockchainDB *db = cryptonote::new_db();

  db->open("./lmdb", cryptonote::TESTNET, DBF_RDONLY);
  std::cout << "opened db, height: " << db->height() << "\n";

  auto ctor = std::make_shared<wallet::TransactionConstructor>();

  crypto::secret_key spend_priv;
  tools::hex_to_type<crypto::secret_key>("d6a2eac72d1432fb816793aa7e8e86947116ac1423cbad5804ca49893e03b00c", spend_priv);
  crypto::public_key spend_pub;
  tools::hex_to_type<crypto::public_key>("2fc259850413006e39450de23e3c63e69ccbdd3a14329707db55e3501bcda5fb", spend_pub);

  crypto::secret_key view_priv;
  tools::hex_to_type<crypto::secret_key>("e93c833da9342958aff37c030cadcd04df8976c06aa2e0b83563205781cb8a02", view_priv);
  crypto::public_key view_pub;
  tools::hex_to_type<crypto::public_key>("5c1e8d44b4d7cb1269e69180dbf7aaf9c1fed4089b2bd4117dd1a70e90f19600", view_pub);

  auto keyring = std::make_shared<wallet::Keyring>(spend_priv, spend_pub, view_priv, view_pub);

  auto wallet = wallet::Wallet::MakeWallet(keyring, ctor, nullptr, ":memory:", "");

  std::cout << "starting parsing from height 664000\n";
  for (size_t i = 664000; i < db->height(); i++)
  {
    auto b = db->get_block_from_height(i);
    auto block_hash = db->get_block_hash_from_height(i);
    std::vector<cryptonote::transaction> txs;
    for (const auto& h : b.tx_hashes)
    {
      txs.push_back(db->get_tx(h));
    }

    std::cout << "calling wallet.AddBlock()\n";

    wallet->AddBlock(b, txs, block_hash, i);

    std::cout << "after block " << i << ", balance is: " << wallet->GetBalance() << "\n";
  }
}
