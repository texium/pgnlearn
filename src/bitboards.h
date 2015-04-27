#ifndef BITBOARDS_H
#define BITBOARDS_H

#include "types.h"

extern U64 BB_SINGLE[64];
extern U64 BB_DIR[64][8];
extern U64 BB_BETWEEN[64][64];

extern U64 BB_PAWN_ATTACKS[64][2];
extern U64 BB_KNIGHT_ATTACKS[64];
extern U64 BB_BISHOP_ATTACKS[64];
extern U64 BB_ROOK_ATTACKS[64];
extern U64 BB_QUEEN_ATTACKS[64];
extern U64 BB_KING_ATTACKS[64];

extern FLD NEXT[64][8];

void InitBitboards();
void Print(U64 b);

inline U64 Up(U64 b)    { return b << 8; }
inline U64 Down(U64 b)  { return b >> 8; }
inline U64 Left(U64 b)  { return (b & LL(0x7f7f7f7f7f7f7f7f)) << 1; }
inline U64 Right(U64 b) { return (b & LL(0xfefefefefefefefe)) >> 1; }

inline U64 UpLeft(U64 b)    { return (b & LL(0x007f7f7f7f7f7f7f)) << 9; }
inline U64 UpRight(U64 b)   { return (b & LL(0x00fefefefefefefe)) << 7; }
inline U64 DownLeft(U64 b)  { return (b & LL(0x7f7f7f7f7f7f7f00)) >> 7; }
inline U64 DownRight(U64 b) { return (b & LL(0xfefefefefefefe00)) >> 9; }

FLD LSB(U64 b);
FLD PopLSB(U64& b);

FLD MSB(U64 b);
FLD PopMSB(U64& b);

U64 BishopAttacks(FLD f, U64 occ);
U64 RookAttacks(FLD f, U64 occ);
U64 QueenAttacks(FLD f, U64 occ);

#endif

