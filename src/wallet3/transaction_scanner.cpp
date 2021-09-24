#include "transaction_scanner.hpp"

#include <common/string_util.h>

#include <vector>

namespace
{

} // anonymous namespace

namespace wallet
{

  std::vector<Output> TransactionScanner::ScanTransactionReceived(const cryptonote::transaction& tx, const crypto::hash& tx_hash, uint64_t height, uint64_t timestamp)
  {
    const auto tx_public_keys = tx.get_public_keys();

    std::vector<Output> received_outputs;

    if (tx_public_keys.empty())
    {
      LOG_PRINT_L0("TransactionScanner found no tx public keys in transaction with hash <" << tx_hash << ">.");
      return {};
    }

    // simple case first, handle only first found tx pubkey
    // TODO: handle all tx pub keys

    // Derivation = a*R where
    //      `a` is the private view key of the recipient
    //      `R` is the tx public key for the output
    //
    //      For standard address:
    //          `R` = `r*G` for random `r`
    //
    //      For subaddress:
    //          `R` = `s*D` for random `s`, `D` = recipient public spend key

    auto derivations = wallet_keys->generate_key_derivations(tx_public_keys);


    // Output belongs to public key derived as follows:
    //      let `Hs` := hash_to_scalar
    //      let `B`  := recipient public spend key
    //      `out_key = Hs(R || output_index)*G + B`
    // 
    // Output belongs to us if we have a public key B such that
    //      `out_key - Hs(R || output_index) * G == B`
    for (size_t output_index = 0; output_index < tx.vout.size(); output_index++)
    {
      const auto& output = tx.vout[output_index];

      if (auto* output_target = std::get_if<cryptonote::txout_to_key>(&output.target))
      {
        size_t derivation_index = 0;
        std::optional<cryptonote::subaddress_index> sub_index{std::nullopt};
        for (derivation_index = 0; derivation_index < derivations.size(); derivation_index++)
        {
          sub_index = wallet_keys->output_and_derivation_ours(derivations[derivation_index], output_target->key, output_index);
          if (sub_index) break;
        }

        if (not sub_index) continue; // not ours, move on to the next output

        //TODO: device "conceal derivation" as needed

        auto key_image = wallet_keys->key_image(derivations[derivation_index], output_target->key, output_index, *sub_index);

        Output o;

        // TODO: ringct mask returned by reference.  ugh.
        auto amount = wallet_keys->output_amount(tx.rct_signatures, derivations[derivation_index], output_index, o.rct_mask);

        o.key_image = key_image;
        o.subaddress_index = *sub_index;
        o.output_index = output_index;
        o.tx_hash = tx_hash;
        o.block_height = height;
        o.block_time = timestamp;

        received_outputs.push_back(std::move(o));
      }
      else
      {
        throw std::invalid_argument("Invalid output target variant, only txout_to_key is valid.");
      }

    }


    return received_outputs;
  }

  std::vector<Output> TransactionScanner::ScanTransactionSpent(const cryptonote::transaction& tx, const crypto::hash& tx_hash, uint64_t height, uint64_t timestamp)
  {
    return {};
  }

} // namespace wallet
