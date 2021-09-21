#include "keyring.hpp"

namespace wallet
{

  crypto::key_derivation Keyring::generate_key_derivation(const crypto::public_key &tx_pubkey)
  {
    return crypto::generate_key_derivation(pub, view_private_key);
  }

}
