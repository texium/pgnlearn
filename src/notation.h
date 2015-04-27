#ifndef NOTATION_H
#define NOTATION_H

#include "moves.h"
#include "position.h"

string FldToStr(FLD f);
string MoveToStr(Move mv);
string MoveToStrShort(Move mv, Position& pos, MoveList* mvlist);
FLD    StrToFld(const string& str);
Move   StrToMove(const string& str, Position& pos);

#endif

