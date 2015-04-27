#include "bitboards.h"

// #define USE_MAGIC_LSB

U64 BB_SINGLE[64];
U64 BB_DIR[64][8];
U64 BB_BETWEEN[64][64];

U64 BB_PAWN_ATTACKS[64][2];
U64 BB_KNIGHT_ATTACKS[64];
U64 BB_BISHOP_ATTACKS[64];
U64 BB_ROOK_ATTACKS[64];
U64 BB_QUEEN_ATTACKS[64];
U64 BB_KING_ATTACKS[64];

FLD NEXT[64][8];

U8 LSB_16[65536];
U8 MSB_16[65536];

U64 BishopAttacks(FLD f, U64 occ)
{
  U64 x = BB_BISHOP_ATTACKS[f];
  U64 y;

  y = BB_DIR[f][1] & occ;
  if (y) x ^= BB_DIR[LSB(y)][1];

  y = BB_DIR[f][3] & occ;
  if (y) x ^= BB_DIR[LSB(y)][3];

  y = BB_DIR[f][5] & occ;
  if (y) x ^= BB_DIR[MSB(y)][5];

  y = BB_DIR[f][7] & occ;
  if (y) x ^= BB_DIR[MSB(y)][7];

  return x;
}
////////////////////////////////////////////////////////////////////////////////


void InitBitboards()
{
  U64 x, y;
  FLD f, from, to;
  int row0, col0, row1, col1;

  x = LL(0x8000000000000000);
  for (f = 0; f < 64; ++f)
  {
    BB_SINGLE[f] = x;
    x >>= 1;
  }

  for (from = 0; from < 64; ++from)
  {
    for (to = 0; to < 64; ++to)
    {
      BB_BETWEEN[from][to] = 0;
    }

    x = BB_SINGLE[from];
    y = 0;
    y |= UpRight(x);
    y |= UpLeft(x);
    BB_PAWN_ATTACKS[from][WHITE] = y;

    x = BB_SINGLE[from];
    y = 0;
    y |= DownRight(x);
    y |= DownLeft(x);
    BB_PAWN_ATTACKS[from][BLACK] = y;

    x = BB_SINGLE[from];
    y = 0;
    y |= Right(UpRight(x));
    y |= Up(UpRight(x));
    y |= Up(UpLeft(x));
    y |= Left(UpLeft(x));
    y |= Left(DownLeft(x));
    y |= Down(DownLeft(x));
    y |= Down(DownRight(x));
    y |= Right(DownRight(x));
    BB_KNIGHT_ATTACKS[from] = y;

    #define ADD_DIR(Shift, delta, dir) \
    x = Shift(BB_SINGLE[from]);        \
    y = 0;                             \
    to = from + delta;                 \
    while (x)                          \
    {                                  \
      BB_BETWEEN[from][to] = y;        \
      y |= x;                          \
      x = Shift(x);                    \
      to += delta;                     \
    }                                  \
    BB_DIR[from][dir] = y;

    ADD_DIR(Right,     1, 0)
    ADD_DIR(UpRight,  -7, 1)
    ADD_DIR(Up,       -8, 2)
    ADD_DIR(UpLeft,   -9, 3)
    ADD_DIR(Left,     -1, 4)
    ADD_DIR(DownLeft,  7, 5)
    ADD_DIR(Down,      8, 6)
    ADD_DIR(DownRight, 9, 7)

    #undef ADD_DIR

    BB_BISHOP_ATTACKS[from] =
      BB_DIR[from][1] |
      BB_DIR[from][3] |
      BB_DIR[from][5] |
      BB_DIR[from][7];

    BB_ROOK_ATTACKS[from] =
      BB_DIR[from][0] |
      BB_DIR[from][2] |
      BB_DIR[from][4] |
      BB_DIR[from][6];

    BB_QUEEN_ATTACKS[from] =
      BB_BISHOP_ATTACKS[from] |
      BB_ROOK_ATTACKS[from];

    x = BB_SINGLE[from];
    y = 0;
    y |= Right(x);
    y |= UpRight(x);
    y |= Up(x);
    y |= UpLeft(x);
    y |= Left(x);
    y |= DownLeft(x);
    y |= Down(x);
    y |= DownRight(x);
    BB_KING_ATTACKS[from] = y;

    row0 = Row(from);
    col0 = Col(from);

    #define ADD_NEXT(drow, dcol, dir) \
    row1 = row0 + drow; \
    col1 = col0 + dcol; \
    if (row1 >= 0 && row1 <= 7 && col1 >= 0 && col1 <= 7) \
      NEXT[from][dir] = 8 * row1 + col1; \
    else \
      NEXT[from][dir] = NF;

    ADD_NEXT( 0,  1, 0)
    ADD_NEXT(-1,  1, 1)
    ADD_NEXT(-1,  0, 2)
    ADD_NEXT(-1, -1, 3)
    ADD_NEXT( 0, -1, 4)
    ADD_NEXT( 1, -1, 5)
    ADD_NEXT( 1,  0, 6)
    ADD_NEXT( 1,  1, 7)

    #undef ADD_NEXT
  }

  int probe;
  for (int i = 0; i < 65536; ++i)
  {
    probe = 1;
    for (int j = 15; j >= 0; --j)
    {
      if (i & probe)
      {
        LSB_16[i] = j;
        break;
      }
      probe <<= 1;
    }

    probe = 0x8000;
    for (int j = 0; j <= 15; ++j)
    {
      if (i & probe)
      {
        MSB_16[i] = j;
        break;
      }
      probe >>= 1;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////


FLD LSB(U64 b)
{
  assert(b != 0);

#ifdef USE_MAGIC_LSB

  static const FLD LSB_Magic[64] =
  {
    A8, B4, E1, H5, E8, B2, E2, G5,
    D8, H4, F7, G2, A7, E3, C3, F5,
    C8, C4, F1, C7, E7, A3, G6, F3,
    H8, D4, G1, E6, B6, E4, H1, E5,
    B8, A4, F8, D1, C1, G7, B7, B1,
    A2, D7, D2, H6, A1, F6, C6, H3,
    G4, G8, H7, C2, F2, A5, H2, D6,
    D3, A6, B5, B3, G3, C5, D5, F4
  };

  U64 lsb = b ^ (b - 1);
  unsigned int foldedLSB = ((int) lsb) ^ ((int)(lsb >> 32));
  int ind = (foldedLSB * 0x78291ACF) >> (32 - 6); // range is 0..63
  return LSB_Magic[ind];

#else

  U32 low32 = b & LL(0xffffffff);
  if (low32)
  {
    if (low32 & 0xffff)
      return 48 + LSB_16[low32 & 0xffff];
    else
      return 32 + LSB_16[low32 >> 16];
  }
  else
  {
    U32 high32 = (U32)(b >> 32);
    if (high32 & 0xffff)
      return 16 + LSB_16[high32 & 0xffff];
    else
      return LSB_16[high32 >> 16];
  }

#endif
}
////////////////////////////////////////////////////////////////////////////////


FLD MSB(U64 b)
{
  assert(b != 0);

  U32 high32 = (U32)(b >> 32);
  if (high32)
  {
    if (high32 >> 16)
      return MSB_16[high32 >> 16];
    else
      return 16 + MSB_16[high32];
  }
  else
  {
    U32 low32 = (U32)b;
    if (low32 >> 16)
      return 32 + MSB_16[low32 >> 16];
    else
      return 48 + MSB_16[low32];
  }
}
////////////////////////////////////////////////////////////////////////////////


FLD PopLSB(U64& b)
{
  assert(b != 0);

  FLD f = LSB(b);
  b ^= BB_SINGLE[f];
  return f;
}
////////////////////////////////////////////////////////////////////////////////


FLD PopMSB(U64& b)
{
  assert(b != 0);

  FLD f = MSB(b);
  b ^= BB_SINGLE[f];
  return f;
}
////////////////////////////////////////////////////////////////////////////////


void Print(U64 b)
{
  cout << endl;
  for (int f = 0; f < 64; ++f)
  {
    if (b & BB_SINGLE[f])
      cout << " 1";
    else
      cout << " -";

    if (Col(f) == 7) cout << endl;
  }
  cout << endl;
}
////////////////////////////////////////////////////////////////////////////////


U64 QueenAttacks(FLD f, U64 occ)
{
  return BishopAttacks(f, occ) | RookAttacks(f, occ);
}
////////////////////////////////////////////////////////////////////////////////


U64 RookAttacks(FLD f, U64 occ)
{
  U64 x = BB_ROOK_ATTACKS[f];
  U64 y;

  y = BB_DIR[f][0] & occ;
  if (y) x ^= BB_DIR[MSB(y)][0];

  y = BB_DIR[f][2] & occ;
  if (y) x ^= BB_DIR[LSB(y)][2];

  y = BB_DIR[f][4] & occ;
  if (y) x ^= BB_DIR[LSB(y)][4];

  y = BB_DIR[f][6] & occ;
  if (y) x ^= BB_DIR[MSB(y)][6];

  return x;
}
////////////////////////////////////////////////////////////////////////////////

