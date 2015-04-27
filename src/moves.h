#ifndef MOVES_H
#define MOVES_H

#include "position.h"

struct MoveList
{
  Move mv;
  EVAL score;
};

MoveList* GenAllMoves(const Position& pos, MoveList* mvlist);
MoveList* GenCaptures(const Position& pos, MoveList* mvlist);
MoveList* AddSimpleChecks(const Position& pos, MoveList* mvlist);

#endif

