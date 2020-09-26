#include <stdio.h>
#include <stdlib.h>

#include "internals.h"
#include "platform.h"

#define INFINITY (__builtin_inff())
#define NAN (__builtin_nanf(""))
#define castUI(a) ((a).v)
#define castQ8(a)     \
  ({                  \
    union ui32_q8 uA; \
    uA.ui = (a);      \
    uA.q;             \
  })

posit8_t p8_add(posit8_t a, posit8_t b) {
  union ui8_p8 uA, uB;
  uint_fast8_t uiA, uiB;
  union ui8_p8 uZ;

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
  } else if (uiA == 0x80 || uiB == 0x80) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x80;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x80;
#endif
    return uZ.p;
  }

  // different signs
  if ((uiA ^ uiB) >> 7)
    return softposit_subMagsP8(uiA, uiB);
  else
    return softposit_addMagsP8(uiA, uiB);
}

posit8_t p8_sub(posit8_t a, posit8_t b) {
  union ui8_p8 uA, uB;
  uint_fast8_t uiA, uiB;
  union ui8_p8 uZ;

  uA.p = a;
  uiA = uA.ui;
  uB.p = b;
  uiB = uB.ui;

#ifdef SOFTPOSIT_EXACT
  uZ.ui.exact = (uiA.ui.exact & uiB.ui.exact);
#endif

  // infinity
  if (uiA == 0x80 || uiB == 0x80) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x80;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x80;
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
  if (signP8UI(uiA ^ uiB))
    return softposit_addMagsP8(uiA, (-uiB & 0xFF));
  else
    return softposit_subMagsP8(uiA, (-uiB & 0xFF));
}

posit8_t p8_mul(posit8_t pA, posit8_t pB) {
  union ui8_p8 uA, uB, uZ;
  uint_fast8_t uiA, uiB;
  uint_fast8_t regA, fracA, regime, tmp;
  bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
  int_fast8_t kA = 0;
  uint_fast16_t frac16Z;

  uA.p = pA;
  uiA = uA.ui;
  uB.p = pB;
  uiB = uB.ui;

#ifdef SOFTPOSIT_EXACT
  uZ.ui.exact = (uiA.ui.exact & uiB.ui.exact);
#endif
  // NaR or Zero
  if (uiA == 0x80 || uiB == 0x80) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x80;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x80;
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

  signA = signP8UI(uiA);
  signB = signP8UI(uiB);
  signZ = signA ^ signB;

  if (signA) uiA = (-uiA & 0xFF);
  if (signB) uiB = (-uiB & 0xFF);

  regSA = signregP8UI(uiA);
  regSB = signregP8UI(uiB);

  tmp = (uiA << 2) & 0xFF;
  if (regSA) {
    while (tmp >> 7) {
      kA++;
      tmp = (tmp << 1) & 0xFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 7)) {
      kA--;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
  }
  fracA = (0x80 | tmp);

  tmp = (uiB << 2) & 0xFF;
  if (regSB) {
    while (tmp >> 7) {
      kA++;
      tmp = (tmp << 1) & 0xFF;
    }
  } else {
    kA--;
    while (!(tmp >> 7)) {
      kA--;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
  }
  frac16Z = (uint_fast16_t)fracA * (0x80 | tmp);

  rcarry = frac16Z >> 15;  // 1st bit of frac32Z
  if (rcarry) {
    kA++;
    frac16Z >>= 1;
  }

  if (kA < 0) {
    regA = (-kA & 0xFF);
    regSA = 0;
    regime = 0x40 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7F - (0x7F >> regA);
  }

  if (regA > 6) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7F) : (uZ.ui = 0x1);
  } else {
    // remove carry and rcarry bits and shift to correct position
    frac16Z = (frac16Z & 0x3FFF) >> regA;
    fracA = (uint_fast8_t)(frac16Z >> 8);
    bitNPlusOne = (0x80 & frac16Z);
    uZ.ui = packToP8UI(regime, fracA);

    // n+1 frac bit is 1. Need to check if another bit is 1 too if not round to
    // even
    if (bitNPlusOne) {
      (0x7F & frac16Z) ? (bitsMore = 1) : (bitsMore = 0);
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }

  if (signZ) uZ.ui = -uZ.ui & 0xFF;
  return uZ.p;
}

posit8_t p8_div(posit8_t pA, posit8_t pB) {
  union ui8_p8 uA, uB, uZ;
  uint_fast8_t uiA, uiB, fracA, fracB, regA, regime, tmp;
  bool signA, signB, signZ, regSA, regSB, bitNPlusOne = 0, bitsMore = 0, rcarry;
  int_fast8_t kA = 0;
  uint_fast16_t frac16A, frac16Z, rem;
  div_t divresult;

  uA.p = pA;
  uiA = uA.ui;
  uB.p = pB;
  uiB = uB.ui;

  // Zero or infinity
  if (uiA == 0x80 || uiB == 0x80 || uiB == 0) {
#ifdef SOFTPOSIT_EXACT
    uZ.ui.v = 0x80;
    uZ.ui.exact = 0;
#else
    uZ.ui = 0x80;
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

  signA = signP8UI(uiA);
  signB = signP8UI(uiB);
  signZ = signA ^ signB;
  if (signA) uiA = (-uiA & 0xFF);
  if (signB) uiB = (-uiB & 0xFF);
  regSA = signregP8UI(uiA);
  regSB = signregP8UI(uiB);

  tmp = (uiA << 2) & 0xFF;
  if (regSA) {
    while (tmp >> 7) {
      kA++;
      tmp = (tmp << 1) & 0xFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 7)) {
      kA--;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
  }
  fracA = (0x80 | tmp);
  frac16A = fracA << 7;  // hidden bit 2nd bit

  tmp = (uiB << 2) & 0xFF;
  if (regSB) {
    while (tmp >> 7) {
      kA--;
      tmp = (tmp << 1) & 0xFF;
    }
    fracB = (0x80 | tmp);
  } else {
    kA++;
    while (!(tmp >> 7)) {
      kA++;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
    fracB = (0x80 | (0x7F & tmp));
  }

  divresult = div(frac16A, fracB);
  frac16Z = divresult.quot;
  rem = divresult.rem;

  if (frac16Z != 0) {
    rcarry =
        frac16Z >>
        7;  // this is the hidden bit (7th bit) , extreme right bit is bit 0
    if (!rcarry) {
      kA--;
      frac16Z <<= 1;
    }
  }

  if (kA < 0) {
    regA = (-kA & 0xFF);
    regSA = 0;
    regime = 0x40 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7F - (0x7F >> regA);
  }
  if (regA > 6) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7F) : (uZ.ui = 0x1);
  } else {
    // remove carry and rcarry bits and shift to correct position
    frac16Z &= 0x7F;
    fracA = (uint_fast16_t)frac16Z >> (regA + 1);

    bitNPlusOne = (0x1 & (frac16Z >> regA));
    uZ.ui = packToP8UI(regime, fracA);

    // uZ.ui = (uint16_t) (regime) + ((uint16_t) (expA)<< (13-regA)) +
    // ((uint16_t)(fracA));
    if (bitNPlusOne) {
      (((1 << regA) - 1) & frac16Z) ? (bitsMore = 1) : (bitsMore = 0);
      if (rem) bitsMore = 1;
      // n+1 frac bit is 1. Need to check if another bit is 1 too if not round
      // to even
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }
  if (signZ) uZ.ui = -uZ.ui & 0xFF;

  return uZ.p;
}

posit8_t p8_sqrt(posit8_t pA) {
  union ui8_p8 uA;
  uint_fast8_t uiA;

  static const uint8_t p8Sqrt[] = {
      0,   8,   11,  14,  16,  18,  20,  21, 23, 24, 25, 27,  28,  29,  30,
      31,  32,  33,  34,  35,  36,  37,  38, 38, 39, 40, 41,  42,  42,  43,
      44,  45,  45,  46,  47,  47,  48,  49, 49, 50, 51, 51,  52,  52,  53,
      54,  54,  55,  55,  56,  57,  57,  58, 58, 59, 59, 60,  60,  61,  61,
      62,  62,  63,  63,  64,  64,  65,  65, 66, 66, 67, 67,  68,  68,  69,
      69,  70,  70,  70,  71,  71,  72,  72, 72, 73, 73, 74,  74,  74,  75,
      75,  75,  76,  76,  77,  77,  77,  79, 80, 81, 83, 84,  85,  86,  87,
      89,  90,  91,  92,  93,  94,  95,  96, 97, 98, 99, 100, 100, 101, 102,
      103, 105, 108, 110, 112, 114, 115, 120};
  uA.p = pA;
  uiA = uA.ui;

  if (uiA >= 0x80) {
    uA.ui = 0x80;
    return uA.p;
  }
  uA.ui = p8Sqrt[uiA];

  return uA.p;
}

double convertP8ToDouble(posit8_t a) {
  union ui8_p8 uZ;
  double d8;
  uZ.p = a;

  if (uZ.ui == 0) {
    return 0;
  } else if (uZ.ui == 0x7F) {  // maxpos
    return 64;
  } else if (uZ.ui == 0x81) {  //-maxpos
    return -64;
  } else if (uZ.ui == 0x80) {  // NaR
    return NAN;
  }

  bool regS, sign;
  uint_fast8_t reg, shift = 2, frac;
  int_fast8_t k = 0;
  double fraction_max;

  sign = signP8UI(uZ.ui);
  if (sign) uZ.ui = -uZ.ui & 0xFF;
  regS = signregP8UI(uZ.ui);

  uint_fast8_t tmp = (uZ.ui << 2) & 0xFF;
  if (regS) {
    while (tmp >> 7) {
      k++;
      shift++;
      tmp = (tmp << 1) & 0xFF;
    }
    reg = k + 1;
  } else {
    k = -1;
    while (!(tmp >> 7)) {
      k--;
      shift++;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
    reg = -k;
  }
  frac = (tmp & 0x7F) >> shift;

  fraction_max = pow(2, 6 - reg);
  d8 = (double)(pow(2, k) * (1 + ((double)frac / fraction_max)));

  if (sign) d8 = -d8;

  return d8;
}

void checkExtraTwoBitsP8(double f8, double temp, bool* bitsNPlusOne,
                         bool* bitsMore) {
  temp /= 2;
  if (temp <= f8) {
    *bitsNPlusOne = 1;
    f8 -= temp;
  }
  if (f8 > 0) *bitsMore = 1;
}
uint_fast16_t convertFractionP8(double f8, uint_fast8_t fracLength,
                                bool* bitsNPlusOne, bool* bitsMore) {
  uint_fast8_t frac = 0;

  if (f8 == 0)
    return 0;
  else if (f8 == INFINITY)
    return 0x80;

  f8 -= 1;  // remove hidden bit
  if (fracLength == 0)
    checkExtraTwoBitsP8(f8, 1.0, bitsNPlusOne, bitsMore);
  else {
    double temp = 1;
    while (true) {
      temp /= 2;
      if (temp <= f8) {
        f8 -= temp;
        fracLength--;
        frac = (frac << 1) + 1;  // shift in one
        if (f8 == 0) {
          // put in the rest of the bits
          frac <<= (uint_fast8_t)fracLength;
          break;
        }

        if (fracLength == 0) {
          checkExtraTwoBitsP8(f8, temp, bitsNPlusOne, bitsMore);

          break;
        }
      } else {
        frac <<= 1;  // shift in a zero
        fracLength--;
        if (fracLength == 0) {
          checkExtraTwoBitsP8(f8, temp, bitsNPlusOne, bitsMore);
          break;
        }
      }
    }
  }

  // printf("convertfloat: frac:%d f16: %.26f  bitsNPlusOne: %d, bitsMore:
  // %d\n", frac, f16, bitsNPlusOne, bitsMore);

  return frac;
}
posit8_t convertDoubleToP8(double f8) {
  union ui8_p8 uZ;
  bool sign;
  uint_fast8_t reg, frac = 0;
  bool bitNPlusOne = 0, bitsMore = 0;

  (f8 >= 0) ? (sign = 0) : (sign = 1);
  // sign: 1 bit, frac: 8 bits, mantisa: 23 bits
  // sign = a.parts.sign;
  // frac = a.parts.fraction;
  // exp = a.parts.exponent;

  if (f8 == 0) {
    uZ.ui = 0;
    return uZ.p;
  } else if (f8 == INFINITY || f8 == -INFINITY || f8 == NAN) {
    uZ.ui = 0x80;
    return uZ.p;
  } else if (f8 == 1) {
    uZ.ui = 0x40;
    return uZ.p;
  } else if (f8 == -1) {
    uZ.ui = 0xC0;
    return uZ.p;
  } else if (f8 >= 64) {
    // maxpos
    uZ.ui = 0x7F;
    return uZ.p;
  } else if (f8 <= -64) {
    // -maxpos
    uZ.ui = 0x81;
    return uZ.p;
  } else if (f8 <= 0.015625 && !sign) {
    // minpos
    uZ.ui = 0x1;
    return uZ.p;
  } else if (f8 >= -0.015625 && sign) {
    //-minpos
    uZ.ui = 0xFF;
    return uZ.p;
  } else if (f8 > 1 || f8 < -1) {
    if (sign) {
      // Make negative numbers positive for easier computation
      f8 = -f8;
    }
    reg = 1;  // because k = m-1; so need to add back 1
    // minpos
    if (f8 <= 0.015625) {
      uZ.ui = 1;
    } else {
      // regime
      while (f8 >= 2) {
        f8 *= 0.5;
        reg++;
      }

      // rounding off regime bits
      if (reg > 6)
        uZ.ui = 0x7F;
      else {
        int8_t fracLength = 6 - reg;
        frac = convertFractionP8(f8, fracLength, &bitNPlusOne, &bitsMore);
        uint_fast8_t regime = 0x7F - (0x7F >> reg);
        uZ.ui = packToP8UI(regime, frac);
        if (bitNPlusOne) uZ.ui += ((uZ.ui & 1) | bitsMore);
      }
      if (sign) uZ.ui = -uZ.ui & 0xFF;
    }
  } else if (f8 < 1 || f8 > -1) {
    if (sign) {
      // Make negative numbers positive for easier computation
      f8 = -f8;
    }
    reg = 0;

    // regime
    // printf("here we go\n");
    while (f8 < 1) {
      f8 *= 2;
      reg++;
    }
    // rounding off regime bits
    if (reg > 6)
      uZ.ui = 0x1;
    else {
      int_fast8_t fracLength = 6 - reg;
      frac = convertFractionP8(f8, fracLength, &bitNPlusOne, &bitsMore);
      uint_fast8_t regime = 0x40 >> reg;
      uZ.ui = packToP8UI(regime, frac);
      if (bitNPlusOne) uZ.ui += ((uZ.ui & 1) | bitsMore);
    }
    if (sign) uZ.ui = -uZ.ui & 0xFF;

  } else {
    // NaR - for NaN, INF and all other combinations
    uZ.ui = 0x80;
  }
  return uZ.p;
}

void printBinary(uint64_t* s, int size) {
  int i;
  uint64_t number = *s;
  int bitSize = size - 1;
  for (i = 0; i < size; ++i) {
    if (i % 8 == 0) putchar(' ');
    printf("%llu", (number >> (bitSize - i)) & 1);
  }
  printf("\n");
}

posit8_t softposit_subMagsP8(uint_fast8_t uiA, uint_fast8_t uiB) {
  uint_fast8_t regA;
  uint_fast16_t frac16A, frac16B;
  uint_fast8_t fracA = 0, regime, tmp;
  bool sign = 0, regSA, regSB, ecarry = 0, bitNPlusOne = 0, bitsMore = 0;
  int_fast16_t shiftRight;
  int_fast8_t kA = 0;
  union ui8_p8 uZ;

  // Both uiA and uiB are actually the same signs if uiB inherits sign of sub
  // Make both positive
  sign = signP8UI(uiA);
  (sign) ? (uiA = (-uiA & 0xFF)) : (uiB = (-uiB & 0xFF));

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

  regSA = signregP8UI(uiA);
  regSB = signregP8UI(uiB);

  tmp = (uiA << 2) & 0xFF;
  if (regSA) {
    while (tmp >> 7) {
      kA++;
      tmp = (tmp << 1) & 0xFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 7)) {
      kA--;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
  }
  frac16A = (0x80 | tmp) << 7;
  shiftRight = kA;

  tmp = (uiB << 2) & 0xFF;
  if (regSB) {
    while (tmp >> 7) {
      shiftRight--;
      tmp = (tmp << 1) & 0xFF;
    }
  } else {
    shiftRight++;
    while (!(tmp >> 7)) {
      shiftRight++;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
  }
  frac16B = (0x80 | tmp) << 7;

  if (shiftRight >= 14) {
    uZ.ui = uiA;
    if (sign) uZ.ui = -uZ.ui & 0xFFFF;
    return uZ.p;
  } else
    frac16B >>= shiftRight;

  frac16A -= frac16B;

  while ((frac16A >> 14) == 0) {
    kA--;
    frac16A <<= 1;
  }
  ecarry = (0x4000 & frac16A) >> 14;
  if (!ecarry) {
    kA--;
    frac16A <<= 1;
  }

  if (kA < 0) {
    regA = (-kA & 0xFF);
    regSA = 0;
    regime = 0x40 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7F - (0x7F >> regA);
  }

  if (regA > 6) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7F) : (uZ.ui = 0x1);
  } else {
    frac16A = (frac16A & 0x3FFF) >> regA;
    fracA = (uint_fast8_t)(frac16A >> 8);
    bitNPlusOne = (0x80 & frac16A);
    uZ.ui = packToP8UI(regime, fracA);

    if (bitNPlusOne) {
      (0x7F & frac16A) ? (bitsMore = 1) : (bitsMore = 0);
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }
  if (sign) uZ.ui = -uZ.ui & 0xFF;
  return uZ.p;
}

posit8_t softposit_addMagsP8(uint_fast8_t uiA, uint_fast8_t uiB) {
  uint_fast8_t regA;
  uint_fast16_t frac16A, frac16B;
  uint_fast8_t fracA = 0, regime, tmp;
  bool sign, regSA, regSB, rcarry = 0, bitNPlusOne = 0, bitsMore = 0;
  int_fast8_t kA = 0;
  int_fast16_t shiftRight;
  union ui8_p8 uZ;

  sign = signP8UI(
      uiA);  // sign is always positive.. actually don't have to do this.
  if (sign) {
    uiA = -uiA & 0xFF;
    uiB = -uiB & 0xFF;
  }

  if ((int_fast8_t)uiA < (int_fast8_t)uiB) {
    uiA ^= uiB;
    uiB ^= uiA;
    uiA ^= uiB;
  }
  regSA = signregP8UI(uiA);
  regSB = signregP8UI(uiB);

  tmp = (uiA << 2) & 0xFF;
  if (regSA) {
    while (tmp >> 7) {
      kA++;
      tmp = (tmp << 1) & 0xFF;
    }
  } else {
    kA = -1;
    while (!(tmp >> 7)) {
      kA--;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
  }
  frac16A = (0x80 | tmp) << 7;
  shiftRight = kA;

  tmp = (uiB << 2) & 0xFF;
  if (regSB) {
    while (tmp >> 7) {
      shiftRight--;
      tmp = (tmp << 1) & 0xFF;
    }
  } else {
    shiftRight++;
    while (!(tmp >> 7)) {
      shiftRight++;
      tmp = (tmp << 1) & 0xFF;
    }
    tmp &= 0x7F;
  }
  frac16B = (0x80 | tmp) << 7;

  // Manage CLANG (LLVM) compiler when shifting right more than number of bits
  (shiftRight > 7) ? (frac16B = 0)
                   : (frac16B >>= shiftRight);  // frac32B >>= shiftRight

  frac16A += frac16B;

  rcarry = 0x8000 & frac16A;  // first left bit
  if (rcarry) {
    kA++;
    frac16A >>= 1;
  }

  if (kA < 0) {
    regA = (-kA & 0xFF);
    regSA = 0;
    regime = 0x40 >> regA;
  } else {
    regA = kA + 1;
    regSA = 1;
    regime = 0x7F - (0x7F >> regA);
  }

  if (regA > 6) {
    // max or min pos. exp and frac does not matter.
    (regSA) ? (uZ.ui = 0x7F) : (uZ.ui = 0x1);
  } else {
    frac16A = (frac16A & 0x3FFF) >> regA;
    fracA = (uint_fast8_t)(frac16A >> 8);
    bitNPlusOne = (0x80 & frac16A);
    uZ.ui = packToP8UI(regime, fracA);

    // n+1 frac bit is 1. Need to check if another bit is 1 too if not round to
    // even
    if (bitNPlusOne) {
      (0x7F & frac16A) ? (bitsMore = 1) : (bitsMore = 0);
      uZ.ui += (uZ.ui & 1) | bitsMore;
    }
  }
  if (sign) uZ.ui = -uZ.ui & 0xFF;
  return uZ.p;
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