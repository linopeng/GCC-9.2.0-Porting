#include <math.h>

#include "internals.h"
#include "platform.h"

posit16_t i32_to_p16(int32_t iA) {
  int_fast8_t k, log2 = 25;
  union ui16_p16 uZ;
  uint_fast16_t uiA;
  uint_fast32_t expA, mask = 0x02000000, fracA;
  bool sign;

  if (iA <
      -134217728) {  //-2147483648 to -134217729 rounds to P32 value -268435456
    uZ.ui = 0x8001;  //-maxpos
    return uZ.p;
  }

  sign = iA >> 31;
  if (sign) {
    iA = -iA & 0xFFFFFFFF;
  }

  if (iA >
      134217728) {  // 134217729 to 2147483647 rounds to  P32 value 268435456
    uiA = 0x7FFF;   // maxpos
  } else if (iA > 0x02FFFFFF) {
    uiA = 0x7FFE;
  } else if (iA < 2) {
    uiA = (iA << 14);
  } else {
    fracA = iA;
    while (!(fracA & mask)) {
      log2--;
      fracA <<= 1;
    }
    k = log2 >> 1;
    expA = (log2 & 0x1) << (12 - k);
    fracA = (fracA ^ mask);

    uiA = (0x7FFF ^ (0x3FFF >> k)) | expA | (fracA >> (k + 13));
    mask = 0x1000 << k;  // bitNPlusOne
    if (mask & fracA) {
      if (((mask - 1) & fracA) | ((mask << 1) & fracA)) uiA++;
    }
  }
  (sign) ? (uZ.ui = -uiA & 0xFFFF) : (uZ.ui = uiA);
  return uZ.p;
}

posit16_t p16_add(posit16_t a, posit16_t b) {
  union ui16_p16 uA, uB;
  uint_fast16_t uiA, uiB;
  union ui16_p16 uZ;

  uA.p = a;
  uiA = uA.ui;
  uB.p = b;
  uiB = uB.ui;

  // Zero or infinity
  if (uiA == 0 || uiB == 0) {  // Not required but put here for speed
    uZ.ui = uiA | uiB;
    return uZ.p;
  } else if (uiA == 0x8000 || uiB == 0x8000) {
    uZ.ui = 0x8000;
    return uZ.p;
  }

  // different signs
  if ((uiA ^ uiB) >> 15) {
    return softposit_subMagsP16(uiA, uiB);
  } else {
    return softposit_addMagsP16(uiA, uiB);
  }
}

posit16_t p16_div(posit16_t pA, posit16_t pB) {
  union ui16_p16 uA, uB, uZ;
  uint_fast16_t uiA, uiB, fracA, fracB, regA, regime, tmp;
  bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
  int_fast8_t expA, kA = 0;
  uint_fast32_t frac32A, frac32Z, rem;
  div_t divresult;

  uA.p = pA;
  uiA = uA.ui;
  uB.p = pB;
  uiB = uB.ui;

  // Zero or infinity
  if (uiA == 0x8000 || uiB == 0x8000 || uiB == 0) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x8000;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x8000;
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

  signA = signP16UI(uiA);
  signB = signP16UI(uiB);
  signZ = signA ^ signB;
  if (signA) uiA = (-uiA & 0xFFFF);
  if (signB) uiB = (-uiB & 0xFFFF);
  regSA = signregP16UI(uiA);
  regSB = signregP16UI(uiB);

  tmp = (uiA << 2) & 0xFFFF;
  if (regSA) {
    while (tmp >> 15) {
      kA++;
      tmp = (tmp << 1) & 0xFFFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 15)) {
      kA--;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
  }
  expA = tmp >> 14;
  fracA = (0x4000 | tmp);
  frac32A = fracA << 14;

  tmp = (uiB << 2) & 0xFFFF;
  if (regSB) {
    while (tmp >> 15) {
      kA--;
      tmp = (tmp << 1) & 0xFFFF;
    }
    fracB = (0x4000 | tmp);
  } else {
    kA++;
    while (!(tmp >> 15)) {
      kA++;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
    fracB = (0x4000 | (0x7FFF & tmp));
  }
  expA -= tmp >> 14;

  divresult = div(frac32A, fracB);
  frac32Z = divresult.quot;
  rem = divresult.rem;

  if (expA < 0) {
    expA = 1;
    kA--;
  }
  if (frac32Z != 0) {
    rcarry =
        frac32Z >>
        14;  // this is the hidden bit (14th bit) , extreme right bit is bit 0
    if (!rcarry) {
      if (expA == 0) kA--;
      expA ^= 1;
      frac32Z <<= 1;
    }
  }
  if (kA < 0) {
    regA = (-kA & 0xFFFF);
    regSA = 0;
    regime = 0x4000 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7FFF - (0x7FFF >> regA);
  }

  if (regA > 14) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7FFF) : (uZ.ui = 0x1);
  } else {
    // remove carry and rcarry bits and shift to correct position
    frac32Z &= 0x3FFF;
    fracA = (uint_fast16_t)frac32Z >> (regA + 1);

    if (regA != 14)
      bitNPlusOne = (frac32Z >> regA) & 0x1;
    else if (fracA > 0) {
      fracA = 0;
      bitsMore = 1;
    }
    if (regA == 14 && expA) bitNPlusOne = 1;

    // sign is always zero
    uZ.ui = packToP16UI(regime, regA, expA, fracA);

    if (bitNPlusOne) {
      (((1 << regA) - 1) & frac32Z) ? (bitsMore = 1) : (bitsMore = 0);
      if (rem) bitsMore = 1;
      // n+1 frac bit is 1. Need to check if another bit is 1 too if not round
      // to even
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }
  if (signZ) uZ.ui = -uZ.ui & 0xFFFF;

  return uZ.p;
}

posit16_t p16_mul(posit16_t pA, posit16_t pB) {
  union ui16_p16 uA, uB, uZ;
  uint_fast16_t uiA, uiB;
  uint_fast16_t regA, fracA, regime, tmp;
  bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
  int_fast8_t expA;
  int_fast8_t kA = 0;
  uint_fast32_t frac32Z;

  uA.p = pA;
  uiA = uA.ui;
  uB.p = pB;
  uiB = uB.ui;

  // NaR or Zero
  if (uiA == 0x8000 || uiB == 0x8000) {
    uZ.ui = 0x8000;
    return uZ.p;
  } else if (uiA == 0 || uiB == 0) {
    uZ.ui = 0;
    return uZ.p;
  }

  signA = signP16UI(uiA);
  signB = signP16UI(uiB);
  signZ = signA ^ signB;

  if (signA) uiA = (-uiA & 0xFFFF);
  if (signB) uiB = (-uiB & 0xFFFF);

  regSA = signregP16UI(uiA);
  regSB = signregP16UI(uiB);

  tmp = (uiA << 2) & 0xFFFF;
  if (regSA) {
    while (tmp >> 15) {
      kA++;
      tmp = (tmp << 1) & 0xFFFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 15)) {
      kA--;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
  }
  expA = tmp >> 14;
  fracA = (0x4000 | tmp);

  tmp = (uiB << 2) & 0xFFFF;
  if (regSB) {
    while (tmp >> 15) {
      kA++;
      tmp = (tmp << 1) & 0xFFFF;
    }
  } else {
    kA--;
    while (!(tmp >> 15)) {
      kA--;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
  }
  expA += tmp >> 14;
  frac32Z = (uint_fast32_t)fracA * (0x4000 | tmp);

  if (expA > 1) {
    kA++;
    expA ^= 0x2;
  }

  rcarry = frac32Z >> 29;  // 3rd bit of frac32Z
  if (rcarry) {
    if (expA) kA++;
    expA ^= 1;
    frac32Z >>= 1;
  }

  if (kA < 0) {
    regA = (-kA & 0xFFFF);
    regSA = 0;
    regime = 0x4000 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7FFF - (0x7FFF >> regA);
  }

  if (regA > 14) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7FFF) : (uZ.ui = 0x1);
  } else {
    // remove carry and rcarry bits and shift to correct position
    frac32Z = (frac32Z & 0xFFFFFFF) >> (regA - 1);
    fracA = (uint_fast16_t)(frac32Z >> 16);

    if (regA != 14)
      bitNPlusOne |= (0x8000 & frac32Z);
    else if (fracA > 0) {
      fracA = 0;
      bitsMore = 1;
    }
    if (regA == 14 && expA) bitNPlusOne = 1;

    // sign is always zero
    uZ.ui = packToP16UI(regime, regA, expA, fracA);
    // n+1 frac bit is 1. Need to check if another bit is 1 too if not round to
    // even
    if (bitNPlusOne) {
      (0x7FFF & frac32Z) ? (bitsMore = 1) : (bitsMore = 0);
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }

  if (signZ) uZ.ui = -uZ.ui & 0xFFFF;
  return uZ.p;
}

posit16_t p16_sub(posit16_t a, posit16_t b) {
  union ui16_p16 uA, uB;
  uint_fast16_t uiA, uiB;
  union ui16_p16 uZ;

  uA.p = a;
  uiA = uA.ui;
  uB.p = b;
  uiB = uB.ui;

#ifdef SOFTPOSIT_EXACT
  uZ.ui.exact = (uiA.ui.exact & uiB.ui.exact);
#endif

  // infinity
  if (uiA == 0x8000 || uiB == 0x8000) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x8000;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x8000;
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
  if ((uiA ^ uiB) >> 15)
    return softposit_addMagsP16(uiA, (-uiB & 0xFFFF));
  else
    return softposit_subMagsP16(uiA, (-uiB & 0xFFFF));
}

int_fast32_t p16_to_i32(posit16_t pA) {
  union ui16_p16 uA;
  int_fast32_t mask, iZ, tmp;
  uint_fast16_t scale = 0, uiA;
  bool bitLast, bitNPlusOne, sign;

  uA.p = pA;
  uiA = uA.ui;  // Copy of the input.
  // NaR
  if (uiA == 0x8000) return 0;

  sign = (uiA > 0x8000);          // sign is True if pA > NaR.
  if (sign) uiA = -uiA & 0xFFFF;  // A is now |A|.

  if (uiA <= 0x3000) {  // 0 <= |pA| <= 1/2 rounds to zero.
    return 0;
  } else if (uiA < 0x4800) {  // 1/2 < x < 3/2 rounds to 1.
    iZ = 1;
  } else if (uiA <= 0x5400) {  // 3/2 <= x <= 5/2 rounds to 2.
    iZ = 2;
  } else {                  // Decode the posit, left-justifying as we go.
    uiA -= 0x4000;          // Strip off first regime bit (which is a 1).
    while (0x2000 & uiA) {  // Increment scale by 2 for each regime sign bit.
      scale += 2;           // Regime sign bit is always 1 in this range.
      uiA =
          (uiA - 0x2000) << 1;  // Remove the bit; line up the next regime bit.
    }
    uiA <<= 1;                  // Skip over termination bit, which is 0.
    if (0x2000 & uiA) scale++;  // If exponent is 1, increment the scale.
    iZ =
        ((uint32_t)uiA | 0x2000)
        << 17;  // Left-justify fraction in 32-bit result (one left bit padding)
    mask = 0x40000000 >> scale;  // Point to the last bit of the integer part.

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

    iZ = (uint32_t)iZ >> (30 - scale);  // Right-justify the integer.
  }

  if (sign) iZ = -iZ;  // Apply the sign of the input.
  return iZ;
}

posit16_t softposit_subMagsP16(uint_fast16_t uiA, uint_fast16_t uiB) {
  uint_fast16_t regA;
  uint_fast32_t frac32A, frac32B;
  uint_fast16_t fracA = 0, regime, tmp;
  bool sign = 0, regSA, regSB, ecarry = 0, bitNPlusOne = 0, bitsMore = 0;
  int_fast16_t shiftRight;
  int_fast8_t kA = 0, expA;
  union ui16_p16 uZ;

  // Both uiA and uiB are actually the same signs if uiB inherits sign of sub
  // Make both positive
  sign = signP16UI(uiA);
  (sign) ? (uiA = (-uiA & 0xFFFF)) : (uiB = (-uiB & 0xFFFF));

  if (uiA == uiB) {  // essential, if not need special handling
    uZ.ui = 0;
    return uZ.p;
  }
  if (uiA < uiB) {
    uiA ^= uiB;
    uiB ^= uiA;
    uiA ^= uiB;
    (sign) ? (sign = 0) : (sign = 1);  // A becomes B
  }

  regSA = signregP16UI(uiA);
  regSB = signregP16UI(uiB);

  tmp = (uiA << 2) & 0xFFFF;
  if (regSA) {
    while (tmp >> 15) {
      kA++;
      tmp = (tmp << 1) & 0xFFFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 15)) {
      kA--;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
  }
  expA = tmp >> 14;
  frac32A = (0x4000 | tmp) << 16;
  shiftRight = kA;

  tmp = (uiB << 2) & 0xFFFF;
  if (regSB) {
    while (tmp >> 15) {
      shiftRight--;
      tmp = (tmp << 1) & 0xFFFF;
    }
  } else {
    shiftRight++;
    while (!(tmp >> 15)) {
      shiftRight++;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
  }
  frac32B = (0x4000 | tmp) << 16;
  // This is 2kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)

  shiftRight = (shiftRight << 1) + expA - (tmp >> 14);

  if (shiftRight != 0) {
    if (shiftRight >= 29) {
      uZ.ui = uiA;
      if (sign) uZ.ui = -uZ.ui & 0xFFFF;
      return uZ.p;
    } else
      frac32B >>= shiftRight;
  }

  frac32A -= frac32B;

  while ((frac32A >> 29) == 0) {
    kA--;
    frac32A <<= 2;
  }
  ecarry = (0x40000000 & frac32A) >> 30;
  if (!ecarry) {
    if (expA == 0) kA--;
    expA ^= 1;
    frac32A <<= 1;
  }

  if (kA < 0) {
    regA = (-kA & 0xFFFF);
    regSA = 0;
    regime = 0x4000 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7FFF - (0x7FFF >> regA);
  }

  if (regA > 14) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7FFF) : (uZ.ui = 0x1);
  } else {
    // remove hidden bits
    frac32A = (frac32A & 0x3FFFFFFF) >> (regA + 1);
    fracA = frac32A >> 16;
    if (regA != 14)
      bitNPlusOne = (frac32A >> 15) & 0x1;
    else if (frac32A > 0) {
      fracA = 0;
      bitsMore = 1;
    }
    if (regA == 14 && expA) bitNPlusOne = 1;
    uZ.ui = packToP16UI(regime, regA, expA, fracA);
    if (bitNPlusOne) {
      (frac32A & 0x7FFF) ? (bitsMore = 1) : (bitsMore = 0);
      // n+1 frac bit is 1. Need to check if another bit is 1 too if not round
      // to even
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }
  if (sign) uZ.ui = -uZ.ui & 0xFFFF;
  return uZ.p;
}

posit16_t softposit_addMagsP16(uint_fast16_t uiA, uint_fast16_t uiB) {
  uint_fast16_t regA, uiX, uiY;
  uint_fast32_t frac32A, frac32B;
  uint_fast16_t fracA = 0, regime, tmp;
  bool sign, regSA, regSB, rcarry = 0, bitNPlusOne = 0, bitsMore = 0;
  int_fast8_t kA = 0, expA;
  int_fast16_t shiftRight;
  union ui16_p16 uZ;

  sign = signP16UI(
      uiA);  // sign is always positive.. actually don't have to do this.
  if (sign) {
    uiA = -uiA & 0xFFFF;
    uiB = -uiB & 0xFFFF;
  }

  if ((int_fast16_t)uiA < (int_fast16_t)uiB) {
    uiX = uiA;
    uiY = uiB;
    uiA = uiY;
    uiB = uiX;
  }
  regSA = signregP16UI(uiA);
  regSB = signregP16UI(uiB);

  tmp = (uiA << 2) & 0xFFFF;
  if (regSA) {
    while (tmp >> 15) {
      kA++;
      tmp = (tmp << 1) & 0xFFFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 15)) {
      kA--;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
  }
  expA = tmp >> 14;
  frac32A = (0x4000 | tmp) << 16;
  shiftRight = kA;

  tmp = (uiB << 2) & 0xFFFF;
  if (regSB) {
    while (tmp >> 15) {
      shiftRight--;
      tmp = (tmp << 1) & 0xFFFF;
    }
    frac32B = (0x4000 | tmp) << 16;
  } else {
    shiftRight++;
    while (!(tmp >> 15)) {
      shiftRight++;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
    frac32B = ((0x4000 | tmp) << 16) & 0x7FFFFFFF;
  }

  // This is 2kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
  shiftRight = (shiftRight << 1) + expA - (tmp >> 14);

  if (shiftRight == 0) {
    frac32A += frac32B;
    // rcarry is one
    if (expA) kA++;
    expA ^= 1;
    frac32A >>= 1;
  } else {
    // Manage CLANG (LLVM) compiler when shifting right more than number of bits
    (shiftRight > 31) ? (frac32B = 0)
                      : (frac32B >>= shiftRight);  // frac32B >>= shiftRight

    frac32A += frac32B;
    rcarry = 0x80000000 & frac32A;  // first left bit
    if (rcarry) {
      if (expA) kA++;
      expA ^= 1;
      frac32A >>= 1;
    }
  }
  if (kA < 0) {
    regA = (-kA & 0xFFFF);
    regSA = 0;
    regime = 0x4000 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7FFF - (0x7FFF >> regA);
  }
  if (regA > 14) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7FFF) : (uZ.ui = 0x1);
  } else {
    // remove hidden bits
    frac32A = (frac32A & 0x3FFFFFFF) >> (regA + 1);
    fracA = frac32A >> 16;
    if (regA != 14)
      bitNPlusOne = (frac32A >> 15) & 0x1;
    else if (frac32A > 0) {
      fracA = 0;
      bitsMore = 1;
    }
    if (regA == 14 && expA) bitNPlusOne = 1;
    uZ.ui = packToP16UI(regime, regA, expA, fracA);
    if (bitNPlusOne) {
      (frac32A & 0x7FFF) ? (bitsMore = 1) : (bitsMore = 0);
      // n+1 frac bit is 1. Need to check if another bit is 1 too if not round
      // to even
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }
  if (sign) uZ.ui = -uZ.ui & 0xFFFF;
  //printf("test: %d\n",uZ.p);
  return uZ.p;
}

double convertP16ToDouble(posit16_t a) {
  union ui16_p16 uZ;
  // posit16 p16;
  double d16;
  uZ.p = a;

  if (uZ.ui == 0) {
    return 0;
  } else if (uZ.ui == 0x7FFF) {  // maxpos -> 32767
    return 268435456;
  } else if (uZ.ui == 0x8001) {  //-maxpos -> 32769
    return -268435456;
  } else if (uZ.ui == 0x8000) {  // NaR -> 32768
    return NAN;
  }

  bool regS, sign;
  uint_fast16_t reg, shift = 2, frac;
  int_fast16_t k = 0;
  int_fast8_t exp;
  double fraction_max;

  sign = signP16UI(uZ.ui);
  if (sign) uZ.ui = -uZ.ui & 0xFFFF;
  regS = signregP16UI(uZ.ui);

  uint_fast16_t tmp = (uZ.ui << 2) & 0xFFFF;
  if (regS) {
    while (tmp >> 15) {
      k++;
      shift++;
      tmp = (tmp << 1) & 0xFFFF;
    }
    reg = k + 1;
  } else {
    k = -1;
    while (!(tmp >> 15)) {
      k--;
      shift++;
      tmp = (tmp << 1) & 0xFFFF;
    }
    tmp &= 0x7FFF;
    reg = -k;
  }
  exp = tmp >> 14;
  frac = (tmp & 0x3FFF) >> shift;

  fraction_max = pow(2, 13 - reg);
  d16 = (double)(pow(4, k) * pow(2, exp) * (1 + ((double)frac / fraction_max)));

  if (sign) d16 = -d16;

  return d16;
}

double func1(double t, int n) {
  double answer = 1;
  for (int i = 0; i < n; i++) answer = answer * t;
  return answer;
}

double func2(double b, int n) {
  double answer = 1;
  for (int i = 1; i <= n; i++) answer = answer * (b - i + 1) / i;
  return answer;
}

double pow(double a, double b) {
  if (a == 0 && b > 0) {
    return 0;
  } else if (a == 0 && b <= 0) {
    return 1 / 0;
  } else if (a < 0 && !(b - (int)b < 0.0001 || (b - (int)b > 0.999))) {
    return 1 / 0;
  }

  if (a <= 2 && a >= 0) {
    double t = a - 1;
    double answer = 1;
    for (int i = 1; i < 100; i++) {
      answer = answer + func1(t, i) * func2(b, i);
    }
    return answer;
  }

  else if (a > 2) {
    int time = 0;

    while (a > 2) {
      a = a / 2;
      time++;
    }

    return pow(a, b) * pow(2, b * time);
  }

  else {
    if ((int)b % 2 == 0) {
      return pow(-a, b);
    } else {
      return -pow(-a, b);
    }
  }
}

void checkExtraTwoBitsP16(double f16, double temp, bool *bitsNPlusOne,
                          bool *bitsMore) {
  temp /= 2;
  if (temp <= f16) {
    *bitsNPlusOne = 1;
    f16 -= temp;
  }
  if (f16 > 0) *bitsMore = 1;
}
uint_fast16_t convertFractionP16(double f16, uint_fast8_t fracLength,
                                 bool *bitsNPlusOne, bool *bitsMore) {
  uint_fast16_t frac = 0;

  if (f16 == 0)
    return 0;
  else if (f16 == INFINITY)
    return 0x8000;

  f16 -= 1;  // remove hidden bit
  if (fracLength == 0)
    checkExtraTwoBitsP16(f16, 1.0, bitsNPlusOne, bitsMore);
  else {
    double temp = 1;
    while (true) {
      temp /= 2;
      if (temp <= f16) {
        f16 -= temp;
        fracLength--;
        frac = (frac << 1) + 1;  // shift in one
        if (f16 == 0) {
          // put in the rest of the bits
          frac <<= (uint_fast8_t)fracLength;
          break;
        }

        if (fracLength == 0) {
          checkExtraTwoBitsP16(f16, temp, bitsNPlusOne, bitsMore);

          break;
        }
      } else {
        frac <<= 1;  // shift in a zero
        fracLength--;
        if (fracLength == 0) {
          checkExtraTwoBitsP16(f16, temp, bitsNPlusOne, bitsMore);
          break;
        }
      }
    }
  }

  return frac;
}
posit16_t convertFloatToP16(float a) { return convertDoubleToP16((double)a); }

posit16_t convertDoubleToP16(double f16) {
  union ui16_p16 uZ;
  bool sign, regS;
  uint_fast16_t reg, frac = 0;
  int_fast8_t exp = 0;
  bool bitNPlusOne = 0, bitsMore = 0;

  (f16 >= 0) ? (sign = 0) : (sign = 1);

  if (f16 == 0) {
    uZ.ui = 0;
    return uZ.p;
  } else if (f16 == INFINITY || f16 == -INFINITY || f16 == NAN) {
    uZ.ui = 0x8000;
    return uZ.p;
  } else if (f16 == 1) {
    uZ.ui = 16384;
    return uZ.p;
  } else if (f16 == -1) {
    uZ.ui = 49152;
    return uZ.p;
  } else if (f16 >= 268435456) {
    // maxpos
    uZ.ui = 32767;
    return uZ.p;
  } else if (f16 <= -268435456) {
    // -maxpos
    uZ.ui = 32769;
    return uZ.p;
  } else if (f16 <= 3.725290298461914e-9 && !sign) {
    // minpos
    uZ.ui = 1;
    return uZ.p;
  } else if (f16 >= -3.725290298461914e-9 && sign) {
    //-minpos
    uZ.ui = 65535;
    return uZ.p;
  } else if (f16 > 1 || f16 < -1) {
    if (sign) {
      // Make negative numbers positive for easier computation
      f16 = -f16;
    }

    regS = 1;
    reg = 1;  // because k = m-1; so need to add back 1
    // minpos
    if (f16 <= 3.725290298461914e-9) {
      uZ.ui = 1;
    } else {
      // regime
      while (f16 >= 4) {
        f16 *= 0.25;
        reg++;
      }
      if (f16 >= 2) {
        f16 *= 0.5;
        exp++;
      }

      int fracLength = 13 - reg;

      if (fracLength < 0) {
        // reg == 14, means rounding bits is exp and just the rest.
        if (f16 > 1) bitsMore = 1;
      } else
        frac = convertFractionP16(f16, fracLength, &bitNPlusOne, &bitsMore);

      if (reg == 14 && frac > 0) {
        bitsMore = 1;
        frac = 0;
      }
      if (reg > 14)
        (regS) ? (uZ.ui = 32767) : (uZ.ui = 0x1);
      else {
        uint_fast16_t regime = 1;
        if (regS) regime = ((1 << reg) - 1) << 1;
        uZ.ui = ((uint16_t)(regime) << (14 - reg)) +
                ((uint16_t)(exp) << (13 - reg)) + ((uint16_t)(frac));
        // n+1 frac bit is 1. Need to check if another bit is 1 too if not round
        // to even
        if (reg == 14 && exp) bitNPlusOne = 1;
        uZ.ui += (bitNPlusOne & (uZ.ui & 1)) | (bitNPlusOne & bitsMore);
      }
      if (sign) uZ.ui = -uZ.ui & 0xFFFF;
    }
  } else if (f16 < 1 || f16 > -1) {
    if (sign) {
      // Make negative numbers positive for easier computation
      f16 = -f16;
    }
    regS = 0;
    reg = 0;

    // regime
    while (f16 < 1) {
      f16 *= 4;
      reg++;
    }
    if (f16 >= 2) {
      f16 /= 2;
      exp++;
    }
    if (reg == 14) {
      bitNPlusOne = exp;
      if (frac > 1) bitsMore = 1;
    } else {
      // only possible combination for reg=15 to reach here is 7FFF (maxpos) and
      // FFFF (-minpos) but since it should be caught on top, so no need to
      // handle
      int_fast8_t fracLength = 13 - reg;
      frac = convertFractionP16(f16, fracLength, &bitNPlusOne, &bitsMore);
    }

    if (reg == 14 && frac > 0) {
      bitsMore = 1;
      frac = 0;
    }
    if (reg > 14)
      (regS) ? (uZ.ui = 32767) : (uZ.ui = 0x1);
    else {
      uint_fast16_t regime = 1;
      if (regS) regime = ((1 << reg) - 1) << 1;
      uZ.ui = ((uint16_t)(regime) << (14 - reg)) +
              ((uint16_t)(exp) << (13 - reg)) + ((uint16_t)(frac));
      // n+1 frac bit is 1. Need to check if another bit is 1 too if not round
      // to even
      if (reg == 14 && exp) bitNPlusOne = 1;
      uZ.ui += (bitNPlusOne & (uZ.ui & 1)) | (bitNPlusOne & bitsMore);
    }
    if (sign) uZ.ui = -uZ.ui & 0xFFFF;
  } else {
    // NaR - for NaN, INF and all other combinations
    uZ.ui = 0x8000;
  }
  return uZ.p;
}

#ifdef SOFTPOSIT_QUAD
void checkQuadExtraTwoBitsP16(__float128 f16, double temp, bool *bitsNPlusOne,
                              bool *bitsMore) {
  temp /= 2;
  if (temp <= f16) {
    *bitsNPlusOne = 1;
    f16 -= temp;
  }
  if (f16 > 0) *bitsMore = 1;
}
uint_fast16_t convertQuadFractionP16(__float128 f16, uint_fast8_t fracLength,
                                     bool *bitsNPlusOne, bool *bitsMore) {
  uint_fast16_t frac = 0;

  if (f16 == 0)
    return 0;
  else if (f16 == INFINITY)
    return 0x8000;

  f16 -= 1;  // remove hidden bit
  if (fracLength == 0)
    checkQuadExtraTwoBitsP16(f16, 1.0, bitsNPlusOne, bitsMore);
  else {
    __float128 temp = 1;
    while (true) {
      temp /= 2;
      if (temp <= f16) {
        f16 -= temp;
        fracLength--;
        frac = (frac << 1) + 1;  // shift in one
        if (f16 == 0) {
          // put in the rest of the bits
          frac <<= (uint_fast8_t)fracLength;
          break;
        }

        if (fracLength == 0) {
          checkQuadExtraTwoBitsP16(f16, temp, bitsNPlusOne, bitsMore);

          break;
        }
      } else {
        frac <<= 1;  // shift in a zero
        fracLength--;
        if (fracLength == 0) {
          checkQuadExtraTwoBitsP16(f16, temp, bitsNPlusOne, bitsMore);
          break;
        }
      }
    }
  }

  return frac;
}

posit16_t convertQuadToP16(__float128 f16) {
  union ui16_p16 uZ;
  bool sign, regS;
  uint_fast16_t reg, frac = 0;
  int_fast8_t exp = 0;
  bool bitNPlusOne = 0, bitsMore = 0;

  (f16 >= 0) ? (sign = 0) : (sign = 1);

  if (f16 == 0) {
    uZ.ui = 0;
    return uZ.p;
  } else if (f16 == INFINITY || f16 == -INFINITY || f16 == NAN) {
    uZ.ui = 0x8000;
    return uZ.p;
  } else if (f16 == 1) {
    uZ.ui = 16384;
    return uZ.p;
  } else if (f16 == -1) {
    uZ.ui = 49152;
    return uZ.p;
  } else if (f16 >= 268435456) {
    // maxpos
    uZ.ui = 32767;
    return uZ.p;
  } else if (f16 <= -268435456) {
    // -maxpos
    uZ.ui = 32769;
    return uZ.p;
  } else if (f16 <= 3.725290298461914e-9 && !sign) {
    // minpos
    uZ.ui = 1;
    return uZ.p;
  } else if (f16 >= -3.725290298461914e-9 && sign) {
    //-minpos
    uZ.ui = 65535;
    return uZ.p;
  } else if (f16 > 1 || f16 < -1) {
    if (sign) {
      // Make negative numbers positive for easier computation
      f16 = -f16;
    }
    regS = 1;
    reg = 1;  // because k = m-1; so need to add back 1
    // minpos
    if (f16 <= 3.725290298461914e-9) {
      uZ.ui = 1;
    } else {
      // regime
      while (f16 >= 4) {
        f16 *= 0.25;
        reg++;
      }
      if (f16 >= 2) {
        f16 *= 0.5;
        exp++;
      }

      int8_t fracLength = 13 - reg;
      if (fracLength < 0) {
        // reg == 14, means rounding bits is exp and just the rest.
        if (f16 > 1) bitsMore = 1;
      } else
        frac = convertQuadFractionP16(f16, fracLength, &bitNPlusOne, &bitsMore);

      if (reg == 14 && frac > 0) {
        bitsMore = 1;
        frac = 0;
      }

      if (reg > 14)
        (regS) ? (uZ.ui = 32767) : (uZ.ui = 0x1);
      else {
        uint_fast16_t regime = 1;
        if (regS) regime = ((1 << reg) - 1) << 1;
        uZ.ui = ((uint16_t)(regime) << (14 - reg)) +
                ((uint16_t)(exp) << (13 - reg)) + ((uint16_t)(frac));
        // n+1 frac bit is 1. Need to check if another bit is 1 too if not round
        // to even
        if (reg == 14 && exp) bitNPlusOne = 1;
        uZ.ui += (bitNPlusOne & (uZ.ui & 1)) | (bitNPlusOne & bitsMore);
      }
      if (sign) uZ.ui = -uZ.ui & 0xFFFF;
    }
  } else if (f16 < 1 || f16 > -1) {
    if (sign) {
      // Make negative numbers positive for easier computation
      f16 = -f16;
    }
    regS = 0;
    reg = 0;

    // regime
    while (f16 < 1) {
      f16 *= 4;
      reg++;
    }
    if (f16 >= 2) {
      f16 /= 2;
      exp++;
    }
    if (reg == 14) {
      bitNPlusOne = exp;
      if (frac > 1) bitsMore = 1;
    } else {
      // only possible combination for reg=15 to reach here is 7FFF (maxpos) and
      // FFFF (-minpos) but since it should be caught on top, so no need to
      // handle
      int_fast8_t fracLength = 13 - reg;
      frac = convertQuadFractionP16(f16, fracLength, &bitNPlusOne, &bitsMore);
    }

    if (reg == 14 && frac > 0) {
      bitsMore = 1;
      frac = 0;
    }
    if (reg > 14)
      (regS) ? (uZ.ui = 32767) : (uZ.ui = 0x1);
    else {
      uint_fast16_t regime = 1;
      if (regS) regime = ((1 << reg) - 1) << 1;
      uZ.ui = ((uint16_t)(regime) << (14 - reg)) +
              ((uint16_t)(exp) << (13 - reg)) + ((uint16_t)(frac));
      // n+1 frac bit is 1. Need to check if another bit is 1 too if not round
      // to even
      if (reg == 14 && exp) bitNPlusOne = 1;
      uZ.ui += (bitNPlusOne & (uZ.ui & 1)) | (bitNPlusOne & bitsMore);
    }
    if (sign) uZ.ui = -uZ.ui & 0xFFFF;
  } else {
    // NaR - for NaN, INF and all other combinations
    uZ.ui = 0x8000;
  }
  return uZ.p;
}
#endif

void printBinary(uint64_t *s, int size) {
  int i;
  uint64_t number = *s;
  int bitSize = size - 1;
  for (i = 0; i < size; ++i) {
    if (i % 8 == 0) putchar(' ');
    printf("%llu", (number >> (bitSize - i)) & 1);
  }
  printf("\n");
}