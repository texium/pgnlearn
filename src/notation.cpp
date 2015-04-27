#include "notation.h"

string FldToStr(FLD f)
{
  char buf[3];

  buf[0] = 'a' + Col(f);
  buf[1] = '8' - Row(f);
  buf[2] = 0;

  return buf;
}
////////////////////////////////////////////////////////////////////////////////


string MoveToStr(Move mv)
{
  string s = FldToStr(mv.From()) + FldToStr(mv.To());

  switch (mv.Promotion())
  {
    case QW: case QB: s += "q"; break;
    case RW: case RB: s += "r"; break;
    case BW: case BB: s += "b"; break;
    case NW: case NB: s += "n"; break;
    default: break;
  }

  return s;
}
////////////////////////////////////////////////////////////////////////////////


string MoveToStrShort(Move mv, Position& pos, MoveList* mvlist)
{
  if (mv.Piece() == KW && mv.From() == E1 && mv.To() == G1) return "O-O";
  if (mv.Piece() == KW && mv.From() == E1 && mv.To() == C1) return "O-O-O";
  if (mv.Piece() == KB && mv.From() == E8 && mv.To() == G8) return "O-O";
  if (mv.Piece() == KB && mv.From() == E8 && mv.To() == C8) return "O-O-O";

  FLD from = mv.From();
  FLD to = mv.To();
  PIECE piece = mv.Piece();
  PIECE captured = mv.Captured();
  PIECE promotion = mv.Promotion();

  string strPiece, strFrom, strTo, strCapture, strPromotion;
  switch (piece)
  {
    case PW: case PB: break;
    case NW: case NB: strPiece = "N"; break;
    case BW: case BB: strPiece = "B"; break;
    case RW: case RB: strPiece = "R"; break;
    case QW: case QB: strPiece = "Q"; break;
    case KW: case KB: strPiece = "K"; break;
    default: break;
  }

  strTo = FldToStr(to);
  if (captured)
  {
    strCapture = "x";
    if (piece == PW || piece == PB)
      strFrom = FldToStr(from).substr(0, 1);
  }

  switch (promotion)
  {
    case QW: case QB: strPromotion += "=Q"; break;
    case RW: case RB: strPromotion += "=R"; break;
    case BW: case BB: strPromotion += "=B"; break;
    case NW: case NB: strPromotion += "=N"; break;
  }

  bool ambiguity = false;
  bool uniqRow = true;
  bool uniqCol = true;

  if (piece != PW && piece != PB)
  {
    for (MoveList* ptr = mvlist; ptr->mv; ++ptr)
    {
      Move mv1 = ptr->mv;

      if (mv1 == mv) continue;
      if (mv1.To() != to) continue;
      if (mv1.Piece() != piece) continue;

      if (!pos.MakeMove(mv1)) continue;
      pos.UnmakeMove();

      ambiguity = true;
      if (Row(mv1.From()) == Row(from)) uniqRow = false;
      if (Col(mv1.From()) == Col(from)) uniqCol = false;
    }
  }

  if (ambiguity)
  {
    if (uniqCol)
      strFrom = FldToStr(from).substr(0, 1);
    else if (uniqRow)
      strFrom = FldToStr(from).substr(1, 1);
    else
      strFrom = FldToStr(from);
  }

  return strPiece + strFrom + strCapture + strTo + strPromotion;
}
////////////////////////////////////////////////////////////////////////////////


FLD StrToFld(const string& str)
{
  if (str.length() < 2) return NF;

  int col = str[0] - 'a';
  int row = '8' - str[1];
  
  if (row >= 0 && row <= 7 && col >= 0 && col <= 7)
    return 8 * row + col;
  else
    return NF;
}
////////////////////////////////////////////////////////////////////////////////


Move StrToMove(const string& str, Position& pos)
{
  if (str.length() < 2) return 0;

  string s = str;
  size_t len = s.length();
  while ((len > 1) &&
         (s[len - 1] == '+' ||
         s[len - 1] == '#' ||
         s[len - 1] == '!' ||
         s[len - 1] == '?'))
  {
    s = s.substr(0, len - 1);
    --len;
  }
  if (len < 2) return 0;

  MoveList mvlist[256];
  GenAllMoves(pos, mvlist);

  // LONG NOTATION

  for (MoveList* ptr = mvlist; ptr->mv; ++ptr)
  {
    Move mv = ptr->mv;
    if (MoveToStr(mv) == s)
    {
      if (pos.MakeMove(mv))
      {
        pos.UnmakeMove();
        return mv;
      }
    }
  }

  // SHORT NOTATION
 
  COLOR side = pos.Side();
  PIECE piece;

  if (str.find("N") == 0)
    piece = NW | side;
  else if (str.find("B") == 0)
    piece = BW | side;
  else if (str.find("R") == 0)
    piece = RW | side;
  else if (str.find("Q") == 0)
    piece = QW | side;
  else if (str.find("K") == 0 || str == "O-O" || str == "O-O-O")
    piece = KW | side;
  else
    piece = PW | side;

  for (MoveList* ptr = mvlist; ptr->mv; ++ptr)
  {
    Move mv = ptr->mv;
    if (mv.Piece() != piece) continue;
    if (MoveToStrShort(mv, pos, mvlist) == s)
    {
      if (pos.MakeMove(mv))
      {
        pos.UnmakeMove();
        return mv;
      }
    }
  }

  return 0;
}
////////////////////////////////////////////////////////////////////////////////

