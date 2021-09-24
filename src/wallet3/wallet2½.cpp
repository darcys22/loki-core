#include "wallet2½.hpp"

#include <ringct/rctSigs.h>

namespace wallet2½
{
  // TODO: copied from wallet2 for now, so to not redo crypto stuff just yet.
  //
  // Definitely quite nasty and needs revisited.
  uint64_t
  output_amount(
      const rct::rctSig& rv,
      const crypto::key_derivation& derivation,
      unsigned int i,
      rct::key& mask,
      hw::device& hwdev)
  {
    crypto::secret_key scalar1;
    hwdev.derivation_to_scalar(derivation, i, scalar1);
    switch (rv.type)
    {
      case rct::RCTType::Simple:
      case rct::RCTType::Bulletproof:
      case rct::RCTType::Bulletproof2:
      case rct::RCTType::CLSAG:
        return rct::decodeRctSimple(rv, rct::sk2rct(scalar1), i, mask, hwdev);
      case rct::RCTType::Full:
        return rct::decodeRct(rv, rct::sk2rct(scalar1), i, mask, hwdev);
      default:
        throw std::invalid_argument("Unsupported rct type");
    }
  }

}  // namespace wallet2½
