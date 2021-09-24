#include "wallet.hpp"

#include "db.hpp"

namespace wallet
{

  Wallet::Wallet(std::shared_ptr<Keyring> _keys,
                 std::shared_ptr<TransactionScanner> _txScanner,
                 std::shared_ptr<TransactionConstructor> _txConstructor,
                 std::shared_ptr<DaemonComms> _daemonComms,
                 std::string_view dbFilename,
                 std::string_view dbPassword)
  {
    try
    {
      db = wallet::OpenDB(dbFilename, dbPassword);
    }
    catch (...)
    {
      // may throw
      db = wallet::CreateDB(dbFilename, dbPassword);
    }
  }

  void Wallet::AddBlock(const cryptonote::block& block, const uint64_t height)
  {
  }

}
