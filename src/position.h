#ifndef POSITION_H
#define POSITION_H

#include "bitboards.h"

extern U64 g_hashFP[64][14];
extern U64 g_hashSide[2];
extern U64 g_hashCastlings[16];
extern U64 g_hashEP[64];

void InitHashNumbers();

enum
{
  WHITE_O_O   = 0x01,
  WHITE_O_O_O = 0x02,
  BLACK_O_O   = 0x10,
  BLACK_O_O_O = 0x20
};


class Move
{
public:
  Move() : m_data(0) {}
  Move(U32 x) : m_data(x) {}
  Move(U32 from, U32 to, U32 piece) :
    m_data(from | (to << 6) | (piece << 12)) {}
  Move(U32 from, U32 to, U32 piece, U32 captured) :
    m_data(from | (to << 6) | (piece << 12) | (captured << 16)) {}
  Move(U32 from, U32 to, U32 piece, U32 captured, U32 promotion) :
    m_data(from | (to << 6) | (piece << 12) | (captured << 16) | (promotion << 20)) {}

  FLD   From() const { return m_data & 0x3f; }
  FLD   To() const { return (m_data >> 6) & 0x3f; }
  PIECE Piece() const { return (m_data >> 12) & 0x0f; }
  PIECE Captured() const { return (m_data >> 16) & 0x0f; }
  PIECE Promotion() const { return (m_data >> 20) & 0x0f; }

  operator U32() const { return m_data; }

private:
  U32 m_data;
};


class Position
{
public:
  U64   Bits(PIECE p) const { return m_bits[p]; }
  U64   BitsAll(COLOR side) const { return m_bitsAll[side]; }
  U64   BitsAll() const { return m_bitsAll[WHITE] | m_bitsAll[BLACK]; }
  U8    Castlings() const { return m_castlings; }
  int   Count(PIECE p) const { return m_count[p]; }
  FLD   EP() const { return m_ep; }
  int   Fifty() const { return m_fifty; }
  U64   Hash() const;
  bool  InCheck() const { return IsAttacked(m_Kings[m_side], m_side ^ 1); }
  bool  IsAttacked(FLD to, COLOR side) const;
  FLD   King(COLOR side) const { return m_Kings[side]; }
  bool  MakeMove(Move mv);
  void  MakeNullMove();
  EVAL  Material(COLOR side) const { return m_material[side]; }
  int   MatIndex(COLOR side) const { return m_matIndex[side]; }
  int   MatSignature(COLOR side) const { return m_matSignature[side]; }
  int   MtblIndex() const;
  int   Ply() const { return m_ply; }
  void  Print() const;
  int   Repetitions() const;
  bool  SetFEN(const string& fen);
  void  SetInitial();
  COLOR Side() const { return m_side; }
  void  UnmakeMove();
  void  UnmakeNullMove();

  PIECE operator[] (FLD f) const { return m_board[f]; }

private:
  void Clear();
  void Put(FLD f, PIECE p);
  void Remove(FLD f);

  PIECE m_board[64];
  U64   m_bits[14];
  U64   m_bitsAll[2];
  U8    m_castlings;
  int   m_count[14];
  FLD   m_ep;
  int   m_fifty;
  U64   m_hashFP;
  FLD   m_Kings[2];
  EVAL  m_material[2];
  int   m_matIndex[2];
  int   m_matSignature[2];
  int   m_ply;
  COLOR m_side;

  struct Undo
  {
    U8   castlings;
    FLD  ep;
    int  fifty;
    U64  hash;
    Move mv;
  };

  Undo m_undos[1024];
  int  m_undoSize;
};

const FLD FLIP[64] =
{
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8
};

#endif

