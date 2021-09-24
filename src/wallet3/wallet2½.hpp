#pragma once

#include <crypto/crypto.h>
#include <ringct/rctTypes.h>
#include <device/device.hpp>

namespace wallet2½
{

  uint64_t output_amount(const rct::rctSig& rv, const crypto::key_derivation& derivation, unsigned int i, rct::key& mask, hw::device &hwdev);

} // namespace wallet2½
