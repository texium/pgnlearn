#include "moves.h"

#define USE_BITBOARDS_FOR_BISHOPS
#define USE_BITBOARDS_FOR_ROOKS
#define USE_BITBOARDS_FOR_QUEENS

MoveList* GenAllMoves(const Position& pos, MoveList* mvlist)
{
  COLOR side = pos.Side();
  COLOR opp = side ^ 1;
  FLD from, to;
  PIECE piece, captured;
  U64 x, y;
  U64 occ = pos.BitsAll();
  U64 freeOrOpp = ~pos.BitsAll(side);

  //
  //   PAWNS
  //

  int fwd = -8 + 16 * side;
  int second = 6 - 5 * side;
  int seventh = 1 + 5 * side;

  piece = PW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
    int row = Row(from);
    to = from + fwd;

    if (!pos[to])
    {
      if (row == second)
      {
        (mvlist++)->mv = Move(from, to, piece);
        to += fwd;
        if (!pos[to])
          (mvlist++)->mv = Move(from, to, piece);
      }
      else if (row == seventh)
      {
        (mvlist++)->mv = Move(from, to, piece, NOPIECE, QW | side);
        (mvlist++)->mv = Move(from, to, piece, NOPIECE, RW | side);
        (mvlist++)->mv = Move(from, to, piece, NOPIECE, BW | side);
        (mvlist++)->mv = Move(from, to, piece, NOPIECE, NW | side);
      }
      else
        (mvlist++)->mv = Move(from, to, piece);
    }

    y = BB_PAWN_ATTACKS[from][side] & pos.BitsAll(opp);
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      if (row == seventh)
      {
        (mvlist++)->mv = Move(from, to, piece, captured, QW | side);
        (mvlist++)->mv = Move(from, to, piece, captured, RW | side);
        (mvlist++)->mv = Move(from, to, piece, captured, BW | side);
        (mvlist++)->mv = Move(from, to, piece, captured, NW | side);
      }
      else
        (mvlist++)->mv = Move(from, to, piece, captured);
    }
  }

  to = pos.EP();
  if (to != NF)
  {
    y = BB_PAWN_ATTACKS[to][opp] & pos.Bits(piece);
    while (y)
    {
      from = PopLSB(y);
      (mvlist++)->mv = Move(from, to, piece, piece ^ 1);
    }
  }

  //
  //   KNIGHTS
  //

  piece = NW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
    y = BB_KNIGHT_ATTACKS[from] & freeOrOpp;
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      (mvlist++)->mv = Move(from, to, piece, captured);
    }
  }

  //
  //   BISHOPS
  //

  piece = BW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
#ifdef USE_BITBOARDS_FOR_BISHOPS
    y = BishopAttacks(from, occ) & freeOrOpp;
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      (mvlist++)->mv = Move(from, to, piece, captured);
    }
#else
    for (int dir = 1; dir < 8; dir += 2)
    {
      for (to = NEXT[from][dir]; to != NF; to = NEXT[to][dir])
      {
        captured = pos[to];
        if (!captured || GetColor(captured) == opp)
          (mvlist++)->mv = Move(from, to, piece, captured);
        if (captured)
          break;
      }
    }
#endif
  }

  //
  //   ROOKS
  //

  piece = RW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
#ifdef USE_BITBOARDS_FOR_ROOKS
    y = RookAttacks(from, occ) & freeOrOpp;
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      (mvlist++)->mv = Move(from, to, piece, captured);
    }
#else
    for (int dir = 0; dir < 8; dir += 2)
    {
      for (to = NEXT[from][dir]; to != NF; to = NEXT[to][dir])
      {
        captured = pos[to];
        if (!captured || GetColor(captured) == opp)
          (mvlist++)->mv = Move(from, to, piece, captured);
        if (captured)
          break;
      }
    }
#endif
  }

  //
  //   QUEENS
  //

  piece = QW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
#ifdef USE_BITBOARDS_FOR_QUEENS
    y = QueenAttacks(from, occ) & freeOrOpp;
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      (mvlist++)->mv = Move(from, to, piece, captured);
    }
#else
    for (int dir = 0; dir < 8; ++dir)
    {
      for (to = NEXT[from][dir]; to != NF; to = NEXT[to][dir])
      {
        captured = pos[to];
        if (!captured || GetColor(captured) == opp)
          (mvlist++)->mv = Move(from, to, piece, captured);
        if (captured)
          break;
      }
    }
#endif
  }

  //
  //   KINGS
  //

  piece = KW | side;
  from = pos.King(side);
  y = BB_KING_ATTACKS[from] & freeOrOpp;
  while (y)
  {
    to = PopLSB(y);
    captured = pos[to];
    (mvlist++)->mv = Move(from, to, piece, captured);
  }

  if (piece == KW && from == E1)
  {
    if (pos.Castlings() & WHITE_O_O)
    {
      if (!pos[F1] && !pos[G1] &&
          !pos.IsAttacked(E1, opp) &&
          !pos.IsAttacked(F1, opp) &&
          !pos.IsAttacked(G1, opp))
      {
        (mvlist++)->mv = Move(E1, G1, KW);
      }
    }

    if (pos.Castlings() & WHITE_O_O_O)
    {
      if (!pos[D1] && !pos[C1] && !pos[B1] &&
          !pos.IsAttacked(E1, opp) &&
          !pos.IsAttacked(D1, opp) &&
          !pos.IsAttacked(C1, opp))
      {
        (mvlist++)->mv = Move(E1, C1, KW);
      }
    }
  }
  else if (piece == KB && from == E8)
  {
    if (pos.Castlings() & BLACK_O_O)
    {
      if (!pos[F8] && !pos[G8] &&
          !pos.IsAttacked(E8, opp) &&
          !pos.IsAttacked(F8, opp) &&
          !pos.IsAttacked(G8, opp))
      {
        (mvlist++)->mv = Move(E8, G8, KB);
      }
    }

    if (pos.Castlings() & BLACK_O_O_O)
    {
      if (!pos[D8] && !pos[C8] && !pos[B8] &&
          !pos.IsAttacked(E8, opp) &&
          !pos.IsAttacked(D8, opp) &&
          !pos.IsAttacked(C8, opp))
      {
        (mvlist++)->mv = Move(E8, C8, KB);
      }
    }
  }

  mvlist->mv = 0;
  return mvlist;
}
////////////////////////////////////////////////////////////////////////////////


MoveList* GenCaptures(const Position& pos, MoveList* mvlist)
{
  COLOR side = pos.Side();
  COLOR opp = side ^ 1;
  FLD from, to;
  PIECE piece, captured;
  U64 x, y;
  U64 occ = pos.BitsAll();

  //
  //   PAWNS
  //

  int fwd = -8 + 16 * side;
  // int second = 6 - 5 * side;
  int seventh = 1 + 5 * side;

  piece = PW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
    int row = Row(from);
    to = from + fwd;

    if (!pos[to])
    {
      if (row == seventh)
      {
        (mvlist++)->mv = Move(from, to, piece, NOPIECE, QW | side);
      }
    }

    y = BB_PAWN_ATTACKS[from][side] & pos.BitsAll(opp);
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      if (row == seventh)
      {
        (mvlist++)->mv = Move(from, to, piece, captured, QW | side);
      }
      else
        (mvlist++)->mv = Move(from, to, piece, captured);
    }
  }

  to = pos.EP();
  if (to != NF)
  {
    y = BB_PAWN_ATTACKS[to][opp] & pos.Bits(piece);
    while (y)
    {
      from = PopLSB(y);
      (mvlist++)->mv = Move(from, to, piece, piece ^ 1);
    }
  }

  //
  //   KNIGHTS
  //

  piece = NW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
    y = BB_KNIGHT_ATTACKS[from] & pos.BitsAll(opp);
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      (mvlist++)->mv = Move(from, to, piece, captured);
    }
  }

  //
  //   BISHOPS
  //

  piece = BW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
#ifdef USE_BITBOARDS_FOR_BISHOPS
    y = BishopAttacks(from, occ) & pos.BitsAll(opp);
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      (mvlist++)->mv = Move(from, to, piece, captured);
    }
#else
    for (int dir = 1; dir < 8; dir += 2)
    {
      for (to = NEXT[from][dir]; to != NF; to = NEXT[to][dir])
      {
        captured = pos[to];
        if (captured && GetColor(captured) == opp)
          (mvlist++)->mv = Move(from, to, piece, captured);
        if (captured)
          break;
      }
    }
#endif
  }

  //
  //   ROOKS
  //

  piece = RW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
#ifdef USE_BITBOARDS_FOR_ROOKS
    y = RookAttacks(from, occ) & pos.BitsAll(opp);
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      (mvlist++)->mv = Move(from, to, piece, captured);
    }
#else
    for (int dir = 0; dir < 8; dir += 2)
    {
      for (to = NEXT[from][dir]; to != NF; to = NEXT[to][dir])
      {
        captured = pos[to];
        if (captured && GetColor(captured) == opp)
          (mvlist++)->mv = Move(from, to, piece, captured);
        if (captured)
          break;
      }
    }
#endif
  }

  //
  //   QUEENS
  //

  piece = QW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
#ifdef USE_BITBOARDS_FOR_QUEENS
    y = QueenAttacks(from, occ) & pos.BitsAll(opp);
    while (y)
    {
      to = PopLSB(y);
      captured = pos[to];
      (mvlist++)->mv = Move(from, to, piece, captured);
    }
#else
    for (int dir = 0; dir < 8; ++dir)
    {
      for (to = NEXT[from][dir]; to != NF; to = NEXT[to][dir])
      {
        captured = pos[to];
        if (captured && GetColor(captured) == opp)
          (mvlist++)->mv = Move(from, to, piece, captured);
        if (captured)
          break;
      }
    }
#endif
  }

  //
  //   KINGS
  //

  piece = KW | side;
  from = pos.King(side);
  y = BB_KING_ATTACKS[from] & pos.BitsAll(opp);
  while (y)
  {
    to = PopLSB(y);
    captured = pos[to];
    (mvlist++)->mv = Move(from, to, piece, captured);
  }

  mvlist->mv = 0;
  return mvlist;
}
////////////////////////////////////////////////////////////////////////////////


MoveList* AddSimpleChecks(const Position& pos, MoveList* mvlist)
{
  COLOR side = pos.Side();
  COLOR opp = side ^ 1;

  FLD K = pos.King(opp);
  FLD from, to;
  PIECE piece;

  U64 occ = pos.BitsAll();
  U64 free = ~occ;
  U64 zoneN = BB_KNIGHT_ATTACKS[K] & free;
  U64 zoneB = BishopAttacks(K, occ) & free;
  U64 zoneR = RookAttacks(K, occ) & free;
  U64 zoneQ = zoneB | zoneR;
  U64 x, y;

  piece = NW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
    y = BB_KNIGHT_ATTACKS[from] & zoneN;
    while (y)
    {
      to = PopLSB(y);
      (mvlist++)->mv = Move(from, to, piece);
    }
  }

  piece = BW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
    y = BishopAttacks(from, occ) & zoneB;
    while (y)
    {
      to = PopLSB(y);
      (mvlist++)->mv = Move(from, to, piece);
    }
  }

  piece = RW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
    y = RookAttacks(from, occ) & zoneR;
    while (y)
    {
      to = PopLSB(y);
      (mvlist++)->mv = Move(from, to, piece);
    }
  }
  
  piece = QW | side;
  x = pos.Bits(piece);
  while (x)
  {
    from = PopLSB(x);
    y = QueenAttacks(from, occ) & zoneQ;
    while (y)
    {
      to = PopLSB(y);
      (mvlist++)->mv = Move(from, to, piece);
    }
  }  
  
  mvlist->mv = 0;
  return mvlist;
}
////////////////////////////////////////////////////////////////////////////////

