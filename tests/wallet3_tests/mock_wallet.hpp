#pragma once

#include <wallet3/wallet.hpp>
#include <sqlitedb/database.hpp>

namespace wallet
{

class MockWallet : public Wallet
{
  public:

    MockWallet() : Wallet({},{},{},{},":memory:",{}){
      //db->prepared_exec("INSERT INTO metadata VALUES(1,0,0,0,0,'',0);");
    };

    int64_t height = 0;
    std::shared_ptr<db::Database> GetDB() { return db; };

    void
    StoreTestTransaction(const int64_t amount) 
    {
      height++;
      std::vector<wallet::Output> dummy_outputs;
      wallet::Output o;
      o.amount = amount;
      o.block_height = height;
      o.subaddress_index = cryptonote::subaddress_index{0,0};
      crypto::key_image key_image;
      *((uint64_t*)&key_image) = height;
      o.key_image = key_image;
      crypto::hash hash = crypto::null_hash;
      std::cout << __FILE__ << ":" << __LINE__ << " (" << __func__ << ") TODO sean remove this - " << "AAAAAAAAAA" << " - debug" << std::endl;
      *((uint64_t*)&hash) = height;
      std::cout << __FILE__ << ":" << __LINE__ << " (" << __func__ << ") TODO sean remove this - " << "AAAAAAAAAA" << " - debug" << std::endl;
      db->prepared_exec("INSERT INTO blocks VALUES(?,?,?);", height, "Adams", 0);
      auto hash_str = tools::type_to_hex(hash);
      std::cout << __FILE__ << ":" << __LINE__ << " (" << __func__ << ") TODO sean remove this - " << "AAAAAAAAAA" << " - debug" << std::endl;
      db->prepared_exec("INSERT INTO transactions(block, hash) VALUES(?,?);", height, hash_str);
      std::cout << __FILE__ << ":" << __LINE__ << " (" << __func__ << ") TODO sean remove this - " << "AAAAAAAAAA" << " - debug" << std::endl;
      db->prepared_exec("INSERT INTO key_images(key_image) VALUES(?);", tools::type_to_hex(key_image));
      std::cout << __FILE__ << ":" << __LINE__ << " (" << __func__ << ") TODO sean remove this - " << "AAAAAAAAAA" << " - debug" << std::endl;
      return StoreTransaction(hash, height, dummy_outputs);
    };
};

} // namespace wallet
