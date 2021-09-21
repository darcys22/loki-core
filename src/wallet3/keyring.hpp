#pragma once

#include <crypto/crypto.h>

namespace wallet
{

  class Keyring
  {
    public:

      Keyring(crypto::secret_key _spend_private_key, 
              crypto::public_key _spend_public_key,
              crypto::secret_key _view_private_key,
              crypto::public_key _view_public_key)
        : spend_private_key(_spend_private_key),
          spend_public_key(_spend_public_key),
          view_private_key(_view_private_key),
          view_public_key(_view_public_key)
    {
    }

    crypto::key_derivation generate_key_derivation(const crypto::public_key &tx_pubkey);

    private:
      crypto::secret_key spend_private_key;
      crypto::public_key spend_public_key;

      crypto::secret_key view_private_key;
      crypto::public_key view_public_key;

      hw::device_default key_device;
  };

} // namespace wallet
