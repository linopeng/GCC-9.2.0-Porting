#include "internals.h"
#include "platform.h"

posit32_t i64_to_p32(int64_t iA) {
  int_fast8_t k, log2 = 63;  // length of bit (e.g. 9222809086901354496) in int
                             // (64 but because we have only 64 bits, so one bit
                             // off to accomdate that fact)
  union ui32_p32 uZ;
  uint_fast64_t uiA;
  uint_fast64_t mask = 0x8000000000000000, fracA;
  uint_fast32_t expA;
  bool sign;

  if (iA <
      -9222809086901354495) {  //-9222809086901354496 to -9223372036854775808
                               //will be P32 value -9223372036854775808
    uZ.ui = 0x80005000;
    return uZ.p;
  }
  sign = iA >> 63;
  if (sign) iA = -iA;

  if (iA > 9222809086901354495)  // 9222809086901354495 bcos 9222809086901354496
                                 // to 9223372036854775807 will be P32 value
                                 // 9223372036854775808
    uiA = 0x7FFFB000;  // P32: 9223372036854775808
  else if (iA < 0x2)
    uiA = (iA << 30);
  else {
    fracA = iA;
    while (!(fracA & mask)) {
      log2--;
      fracA <<= 1;
    }

    k = (log2 >> 2);

    expA = (log2 & 0x3) << (27 - k);
    fracA = (fracA ^ mask);

    uiA = (0x7FFFFFFF ^ (0x3FFFFFFF >> k)) | expA | fracA >> (k + 36);

    mask = 0x800000000 << k;  // bitNPlusOne

    if (mask & fracA) {
      if (((mask - 1) & fracA) | ((mask << 1) & fracA)) uiA++;
    }
  }
  (sign) ? (uZ.ui = -uiA) : (uZ.ui = uiA);
  return uZ.p;
}

posit32_t p32_add(posit32_t a, posit32_t b) {
  union ui32_p32 uA, uB, uZ;
  uint_fast32_t uiA, uiB;

  uA.p = a;
  uiA = uA.ui;
  uB.p = b;
  uiB = uB.ui;

#ifdef SOFTPOSIT_EXACT
  uZ.ui.exact = (uiA.ui.exact & uiB.ui.exact);
#endif

  // Zero or infinity
  if (uiA == 0 || uiB == 0) {  // Not required but put here for speed
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = uiA | uiB;
    uZ.ui.exact = (uiA.ui.exact & uiB.ui.exact);
#else
    uZ.ui = uiA | uiB;
#endif
    return uZ.p;
  } else if (uiA == 0x80000000 || uiB == 0x80000000) {
    // printf("in infinity\n");
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x80000000;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x80000000;
#endif
    return uZ.p;
  }

  // different signs
  if ((uiA ^ uiB) >> 31)
    return softposit_subMagsP32(uiA, uiB);
  else
    return softposit_addMagsP32(uiA, uiB);
}

posit32_t p32_sub(posit32_t a, posit32_t b) {
  union ui32_p32 uA, uB, uZ;
  uint_fast32_t uiA, uiB;

  uA.p = a;
  uiA = uA.ui;
  uB.p = b;
  uiB = uB.ui;

#ifdef SOFTPOSIT_EXACT
  uZ.ui.exact = (uiA.ui.exact & uiB.ui.exact);
#endif

  // infinity
  if (uiA == 0x80000000 || uiB == 0x80000000) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x80000000;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x80000000;
#endif
    return uZ.p;
  }
  // Zero
  else if (uiA == 0 || uiB == 0) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = (uiA | -uiB);
    uZ.ui.exact = 0;
#else
    uZ.ui = (uiA | -uiB);
#endif
    return uZ.p;
  }

  // different signs
  if ((uiA ^ uiB) >> 31)
    return softposit_addMagsP32(uiA, (-uiB & 0xFFFFFFFF));
  else
    return softposit_subMagsP32(uiA, (-uiB & 0xFFFFFFFF));
}

posit32_t p32_mul(posit32_t pA, posit32_t pB) {
  union ui32_p32 uA, uB, uZ;
  uint_fast32_t uiA, uiB;
  uint_fast32_t regA, fracA, regime, tmp;
  bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
  int_fast32_t expA;
  int_fast8_t kA = 0;
  uint_fast64_t frac64Z;

  uA.p = pA;
  uiA = uA.ui;
  uB.p = pB;
  uiB = uB.ui;

#ifdef SOFTPOSIT_EXACT
  uZ.ui.exact = (uiA.ui.exact & uiB.ui.exact);
#endif
  // NaR or Zero
  if (uiA == 0x80000000 || uiB == 0x80000000) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x80000000;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x80000000;
#endif
    return uZ.p;
  } else if (uiA == 0 || uiB == 0) {
#ifdef SOFTPOSIT_EXACT

    uZ.ui.v = 0;
    if ((uiA == 0 && uiA.ui.exact) || (uiB == 0 && uiB.ui.exact))
      uZ.ui.exact = 1;
    else
      uZ.ui.exact = 0;
#else
    uZ.ui = 0;
#endif
    return uZ.p;
  }

  signA = signP32UI(uiA);
  signB = signP32UI(uiB);
  signZ = signA ^ signB;

  if (signA) uiA = (-uiA & 0xFFFFFFFF);
  if (signB) uiB = (-uiB & 0xFFFFFFFF);

  regSA = signregP32UI(uiA);
  regSB = signregP32UI(uiB);

  tmp = (uiA << 2) & 0xFFFFFFFF;
  if (regSA) {
    while (tmp >> 31) {
      kA++;
      tmp = (tmp << 1) & 0xFFFFFFFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 31)) {
      kA--;
      tmp = (tmp << 1) & 0xFFFFFFFF;
    }
    tmp &= 0x7FFFFFFF;
  }
  expA = tmp >> 29;  // to get 2 bits
  fracA = ((tmp << 1) | 0x40000000) & 0x7FFFFFFF;

  tmp = (uiB << 2) & 0xFFFFFFFF;
  if (regSB) {
    while (tmp >> 31) {
      kA++;
      tmp = (tmp << 1) & 0xFFFFFFFF;
    }
  } else {
    kA--;
    while (!(tmp >> 31)) {
      kA--;
      tmp = (tmp << 1) & 0xFFFFFFFF;
    }
    tmp &= 0x7FFFFFFF;
  }
  expA += tmp >> 29;
  frac64Z = (uint_fast64_t)fracA * (((tmp << 1) | 0x40000000) & 0x7FFFFFFF);

  if (expA > 3) {
    kA++;
    expA &= 0x3;  // -=4
  }

  rcarry = frac64Z >> 61;  // 3rd bit of frac64Z
  if (rcarry) {
    expA++;
    if (expA > 3) {
      kA++;
      expA &= 0x3;
    }
    frac64Z >>= 1;
  }

  if (kA < 0) {
    regA = -kA;
    regSA = 0;
    regime = 0x40000000 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7FFFFFFF - (0x7FFFFFFF >> regA);
  }

  if (regA > 30) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7FFFFFFF) : (uZ.ui = 0x1);
  } else {
    // remove carry and rcarry bits and shift to correct position (2 bits exp,
    // so + 1 than 16 bits)
    frac64Z = (frac64Z & 0xFFFFFFFFFFFFFFF) >> regA;
    fracA = (uint_fast32_t)(frac64Z >> 32);

    if (regA <= 28) {
      bitNPlusOne |= (0x80000000 & frac64Z);
      expA <<= (28 - regA);
    } else {
      if (regA == 30) {
        bitNPlusOne = expA & 0x2;
        bitsMore = (expA & 0x1);
        expA = 0;
      } else if (regA == 29) {
        bitNPlusOne = expA & 0x1;
        expA >>= 1;  // taken care of by the pack algo
      }
      if (fracA > 0) {
        fracA = 0;
        bitsMore = 1;
      }
    }
    // sign is always zero
    uZ.ui = packToP32UI(regime, expA, fracA);
    // n+1 frac bit is 1. Need to check if another bit is 1 too if not round to
    // even
    if (bitNPlusOne) {
      (0x7FFFFFFF & frac64Z) ? (bitsMore = 1) : (bitsMore = 0);
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }

  if (signZ) uZ.ui = -uZ.ui & 0xFFFFFFFF;
  return uZ.p;
}

posit32_t p32_div(posit32_t pA, posit32_t pB) {
  union ui32_p32 uA, uB, uZ;
  uint_fast32_t uiA, uiB, fracA, fracB, regA, regime, regB, tmp;
  bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
  int_fast8_t kA = 0;
  int_fast32_t expA;
  uint_fast64_t frac64A, frac64Z, rem;
  lldiv_t divresult;

  uA.p = pA;
  uiA = uA.ui;
  uB.p = pB;
  uiB = uB.ui;

  // Zero or infinity
  if (uiA == 0x80000000 || uiB == 0x80000000 || uiB == 0) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x80000000;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x80000000;
#endif
    return uZ.p;
  } else if (uiA == 0) {
#ifdef SOFTPOSIT_EXACT

    uZ.ui.v = 0;
    if ((uiA == 0 && uiA.ui.exact) || (uiB == 0 && uiB.ui.exact))
      uZ.ui.exact = 1;
    else
      uZ.ui.exact = 0;
#else
    uZ.ui = 0;
#endif
    return uZ.p;
  }

  signA = signP32UI(uiA);
  signB = signP32UI(uiB);
  signZ = signA ^ signB;
  if (signA) uiA = (-uiA & 0xFFFFFFFF);
  if (signB) uiB = (-uiB & 0xFFFFFFFF);
  regSA = signregP32UI(uiA);
  regSB = signregP32UI(uiB);

  tmp = (uiA << 2) & 0xFFFFFFFF;
  if (regSA) {
    while (tmp >> 31) {
      kA++;
      tmp = (tmp << 1) & 0xFFFFFFFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 31)) {
      kA--;
      tmp = (tmp << 1) & 0xFFFFFFFF;
    }
    tmp &= 0x7FFFFFFF;
  }
  expA = tmp >> 29;  // to get 2 bits
  fracA = ((tmp << 1) | 0x40000000) & 0x7FFFFFFF;
  frac64A = (uint64_t)fracA << 30;

  tmp = (uiB << 2) & 0xFFFFFFFF;
  if (regSB) {
    while (tmp >> 31) {
      kA--;
      tmp = (tmp << 1) & 0xFFFFFFFF;
    }
  } else {
    kA++;
    while (!(tmp >> 31)) {
      kA++;
      tmp = (tmp << 1) & 0xFFFFFFFF;
    }
    tmp &= 0x7FFFFFFF;
  }
  expA -= tmp >> 29;
  fracB = ((tmp << 1) | 0x40000000) & 0x7FFFFFFF;

  divresult = lldiv(frac64A, (uint_fast64_t)fracB);
  frac64Z = divresult.quot;
  rem = divresult.rem;

  if (expA < 0) {
    expA += 4;
    kA--;
  }
  if (frac64Z != 0) {
    rcarry =
        frac64Z >>
        30;  // this is the hidden bit (14th bit) , extreme right bit is bit 0
    if (!rcarry) {
      if (expA == 0) {
        kA--;
        expA = 3;
      } else
        expA--;
      frac64Z <<= 1;
    }
  }

  if (kA < 0) {
    regA = -kA;
    regSA = 0;
    regime = 0x40000000 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7FFFFFFF - (0x7FFFFFFF >> regA);
  }
  if (regA > 30) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7FFFFFFF) : (uZ.ui = 0x1);
  } else {
    // remove carry and rcarry bits and shift to correct position
    frac64Z &= 0x3FFFFFFF;

    fracA = (uint_fast32_t)frac64Z >> (regA + 2);

    if (regA <= 28) {
      bitNPlusOne = (frac64Z >> (regA + 1)) & 0x1;
      expA <<= (28 - regA);
      if (bitNPlusOne)
        (((1 << (regA + 1)) - 1) & frac64Z) ? (bitsMore = 1) : (bitsMore = 0);
    } else {
      if (regA == 30) {
        bitNPlusOne = expA & 0x2;
        bitsMore = (expA & 0x1);
        expA = 0;
      } else if (regA == 29) {
        bitNPlusOne = expA & 0x1;
        expA >>= 1;  // taken care of by the pack algo
      }
      if (frac64Z > 0) {
        fracA = 0;
        bitsMore = 1;
      }
    }
    if (rem) bitsMore = 1;

    uZ.ui = packToP32UI(regime, expA, fracA);
    if (bitNPlusOne) uZ.ui += (uZ.ui & 1) | bitsMore;
  }

  if (signZ) uZ.ui = -uZ.ui & 0xFFFFFFFF;
  return uZ.p;
}

int_fast64_t p32_to_i64(posit32_t pA) {
  union ui32_p32 uA;
  uint_fast64_t mask, tmp;
  int_fast64_t iZ;
  uint_fast32_t scale = 0, uiA;
  bool bitLast, bitNPlusOne, bitsMore, sign;

  uA.p = pA;
  uiA = uA.ui;

  if (uiA == 0x80000000) return 0;

  sign = uiA >> 31;
  if (sign) uiA = -uiA & 0xFFFFFFFF;

  if (uiA <= 0x38000000)
    return 0;  // 0 <= |pA| <= 1/2 rounds to zero.
  else if (uiA < 0x44000000)
    iZ = 1;  // 1/2 < x < 3/2 rounds to 1.
  else if (uiA <= 0x4A000000)
    iZ = 2;  // 3/2 <= x <= 5/2 rounds to 2.
  // overflow so return max integer value
  else if (uiA > 0x7FFFAFFF)
    return (sign) ? (-9223372036854775808) : (0x7FFFFFFFFFFFFFFF);
  else {
    uiA -= 0x40000000;
    while (0x20000000 & uiA) {
      scale += 4;
      uiA = (uiA - 0x20000000) << 1;
    }
    uiA <<= 1;  // Skip over termination bit, which is 0.
    if (0x20000000 & uiA)
      scale += 2;  // If first exponent bit is 1, increment the scale.
    if (0x10000000 & uiA) scale++;
    iZ =
        ((uiA | 0x10000000ULL) & 0x1FFFFFFFULL)
        << 34;  // Left-justify fraction in 32-bit result (one left bit padding)

    if (scale < 62) {
      mask = 0x4000000000000000 >>
             scale;  // Point to the last bit of the integer part.

      bitLast = (iZ & mask);  // Extract the bit, without shifting it.
      mask >>= 1;
      tmp = (iZ & mask);
      bitNPlusOne = tmp;      // "True" if nonzero.
      iZ ^= tmp;              // Erase the bit, if it was set.
      tmp = iZ & (mask - 1);  // tmp has any remaining bits. // This is bitsMore
      iZ ^= tmp;              // Erase those bits, if any were set.

      if (bitNPlusOne) {  // logic for round to nearest, tie to even
        if (bitLast | tmp) iZ += (mask << 1);
      }
      iZ = ((uint64_t)iZ) >> (62 - scale);  // Right-justify the integer.
    } else if (scale > 62)
      iZ = (uint64_t)iZ << (scale - 62);
  }

  if (sign) iZ = -iZ;
  return iZ;
}
