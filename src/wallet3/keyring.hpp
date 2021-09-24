#pragma once

#include <crypto/crypto.h>
#include <cryptonote_basic/subaddress_index.h>
#include <device/device_default.hpp>
#include <ringct/rctSigs.h>

#include <optional>

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

    crypto::key_derivation generate_key_derivation(const crypto::public_key &tx_pubkey) const;

    crypto::public_key output_spend_key(const crypto::key_derivation& derivation, const crypto::public_key& output_key, uint64_t output_index);

    std::optional<cryptonote::subaddress_index> output_and_derivation_ours(const crypto::key_derivation& derivation, const crypto::public_key& output_key, uint64_t output_index);

    crypto::key_image key_image(const crypto::key_derivation& derivation, const crypto::public_key& output_key, uint64_t output_index, const cryptonote::subaddress_index& sub_index);

    uint64_t output_amount(const rct::rctSig& rv, const crypto::key_derivation& derivation, unsigned int i, rct::key& mask);

    private:
      crypto::secret_key spend_private_key;
      crypto::public_key spend_public_key;

      crypto::secret_key view_private_key;
      crypto::public_key view_public_key;

      hw::core::device_default key_device;
  };

} // namespace wallet
