#include "eval.h"
#include "notation.h"
#include "position.h"
#include "utils.h"

U64 g_hashFP[64][14];
U64 g_hashSide[2];
U64 g_hashCastlings[16];
U64 g_hashEP[64];

void InitHashNumbers()
{
  RandSeed(1);

  for (FLD f = 0; f < 64; ++f)
    for (PIECE p = 0; p < 14; ++p)
      g_hashFP[f][p] = Rand64();

  g_hashSide[WHITE] = Rand64();
  g_hashSide[BLACK] = Rand64();

  for (int i = 0; i < 16; ++i)
    g_hashCastlings[i] = Rand64();

  for (FLD f = 0; f < 64; ++f)
    g_hashEP[f] = Rand64();
}
////////////////////////////////////////////////////////////////////////////////

void Position::Clear()
{
  for (FLD f = 0; f < 64; ++f)
  {
    m_board[f] = NOPIECE;
  }

  for (PIECE p = 0; p < 14; ++p)
  {
    m_bits[p] = 0;
    m_count[p] = 0;
  }

  m_bitsAll[WHITE] = m_bitsAll[BLACK] = 0;
  m_castlings = 0;
  m_ep = NF;
  m_fifty = 0;
  m_hashFP = 0;
  m_Kings[WHITE] = m_Kings[BLACK] = NF;
  m_material[WHITE] = m_material[BLACK] = 0;
  m_matIndex[WHITE] = m_matIndex[BLACK] = 0;
  m_matSignature[WHITE] = m_matSignature[BLACK] = 0;
  m_ply = 0;
  m_side = WHITE;
  m_undoSize = 0;
}
////////////////////////////////////////////////////////////////////////////////

U64 Position::Hash() const
{
  U64 h = m_hashFP;
  h ^= g_hashSide[m_side];
  h ^= g_hashCastlings[m_castlings & 0xff];
  h ^= (m_ep != NF)? g_hashEP[m_ep] : 0;
  return h;
}
////////////////////////////////////////////////////////////////////////////////

bool Position::IsAttacked(FLD to, COLOR side) const
{
  if (BB_PAWN_ATTACKS[to][side ^ 1] & m_bits[PW | side]) return true;
  if (BB_KNIGHT_ATTACKS[to] & m_bits[NW | side]) return true;
  if (BB_KING_ATTACKS[to] & m_bits[KW | side]) return true;

  U64 x, occ = BitsAll();
  FLD from;

  x = BB_BISHOP_ATTACKS[to] & (m_bits[BW | side] | m_bits[QW | side]);
  while (x)
  {
    from = PopLSB(x);
    if ((BB_BETWEEN[from][to] & occ) == 0) return true;
  }

  x = BB_ROOK_ATTACKS[to] & (m_bits[RW | side] | m_bits[QW | side]);
  while (x)
  {
    from = PopLSB(x);
    if ((BB_BETWEEN[from][to] & occ) == 0) return true;
  }

  return false;
}
////////////////////////////////////////////////////////////////////////////////

bool Position::MakeMove(Move mv)
{
  COLOR side = m_side;
  COLOR opp = side ^ 1;

  Undo& undo = m_undos[m_undoSize++];
  undo.castlings = m_castlings;
  undo.ep = m_ep;
  undo.fifty = m_fifty;
  undo.hash = Hash();
  undo.mv = mv;

  FLD from = mv.From();
  FLD to = mv.To();
  PIECE piece = mv.Piece();
  PIECE captured = mv.Captured();

  ++m_fifty;

  Remove(from);
  if (captured)
  {
    m_fifty = 0;
    if (to == m_ep)
      Remove(to + 8 - 16 * side);
    else
      Remove(to);
  }
  Put(to, piece);

  m_ep = NF;
  switch (piece)
  {
    case PW:
    case PB:
      m_fifty = 0;
      if (to - from == -16 + 32 * side)
        m_ep = to + 8 -  16 * side;
      else if (mv.Promotion())
      {
        Remove(to);
        Put(to, mv.Promotion());
      }
      break;
    case KW:
      m_Kings[WHITE] = to;
      if (from == E1)
      {
        if (to == G1)
        {
          Remove(H1);
          Put(F1, RW);
        }
        else if (to == C1)
        {
          Remove(A1);
          Put(D1, RW);
        }
      }
      break;
    case KB:
      m_Kings[BLACK] = to;
      if (from == E8)
      {
        if (to == G8)
        {
          Remove(H8);
          Put(F8, RB);
        }
        else if (to == C8)
        {
          Remove(A8);
          Put(D8, RB);
        }
      }      
      break;
    default:
      break;
  }

  ++m_ply;
  m_side ^= 1;

  static const U8 DELTA_CASTLINGS[64] =
  {
    0xdf, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xef,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfd, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xfe
  };

  m_castlings &= DELTA_CASTLINGS[from];
  m_castlings &= DELTA_CASTLINGS[to];

  if (IsAttacked(m_Kings[side], opp))
  {
    UnmakeMove();
    return false;
  }

  return true;
}
////////////////////////////////////////////////////////////////////////////////

void Position::MakeNullMove()
{
  Undo& undo = m_undos[m_undoSize++];
  undo.castlings = m_castlings;
  undo.ep = m_ep;
  undo.fifty = m_fifty;
  undo.hash = Hash();
  undo.mv = 0;

  m_ep = NF;
  ++m_ply;
  m_side ^= 1;
}
////////////////////////////////////////////////////////////////////////////////

int Position::MtblIndex() const
{
	int index = 0;

	if (Count(PW) > 8 || Count(PB) > 8) return -1;
	if (Count(NW) > 2 || Count(NB) > 2) return -1;
	if (Count(BW) > 2 || Count(BB) > 2) return -1;
	if (Count(RW) > 2 || Count(RB) > 2) return -1;
	if (Count(QW) > 1 || Count(QB) > 1) return -1;

	index = 8 * index + Count(PW);
	index = 8 * index + Count(PB);
	index = 3 * index + Count(NW);
	index = 3 * index + Count(NB);
	index = 3 * index + Count(BW);
	index = 3 * index + Count(BB);
	index = 3 * index + Count(RW);
	index = 3 * index + Count(RB);
	index = 2 * index + Count(QW);
	index = 2 * index + Count(QB);

	return index;
}
////////////////////////////////////////////////////////////////////////////////

void Position::Print() const
{
  const char names[] = "-?PpNnBbRrQqKk";

  cout << endl;
  for (FLD f = 0; f < 64; ++f)
  {
    PIECE p = m_board[f];
    cout << " " << names[p];
    if (Col(f) == 7) cout << endl;
  }

  if (m_undoSize)
  {
    cout << endl;
    for (int m = 0; m < m_undoSize; ++m)
      cout << " " << MoveToStr(m_undos[m].mv);
    cout << endl;
  }

  cout << endl;
}
////////////////////////////////////////////////////////////////////////////////

const int MAT_SIGNATURE[7] = { 0, 1, 10, 100, 1000, 10000, 0 };
const int MAT_INDEX[14] = { 0, 0, 0, 0, 3, 3, 3, 3, 5, 5, 10, 10, 0, 0 };

void Position::Put(FLD f, PIECE p)
{
  assert(f >= 0 && f <= 63);
  assert(p >= 2 && p <= 13);
  assert(m_board[f] == NOPIECE);

  COLOR side = GetColor(p);
  m_board[f] = p;

  m_bits[p] ^= BB_SINGLE[f];
  m_bitsAll[side] ^= BB_SINGLE[f];

  ++m_count[p];
  m_material[side] += VALUE[p];
  m_matIndex[side] += MAT_INDEX[p];
  m_matSignature[side] += MAT_SIGNATURE[p / 2];

  m_hashFP ^= g_hashFP[f][p];
}
////////////////////////////////////////////////////////////////////////////////

void Position::Remove(FLD f)
{
  assert(f >= 0 && f <= 63);
  PIECE p = m_board[f];
  assert(p >= 2 && p <= 13);

  COLOR side = GetColor(p);
  m_board[f] = NOPIECE;

  m_bits[p] ^= BB_SINGLE[f];
  m_bitsAll[side] ^= BB_SINGLE[f];

  --m_count[p];
  m_material[side] -= VALUE[p];
  m_matIndex[side] -= MAT_INDEX[p];  
  m_matSignature[side] -= MAT_SIGNATURE[p / 2];  

  m_hashFP ^= g_hashFP[f][p];
}
////////////////////////////////////////////////////////////////////////////////

int Position::Repetitions() const
{
  int r = 1;
  U64 hash0 = Hash();
  for (int i = m_undoSize - 1; i >= 0; --i)
  {
    if (m_undos[i].hash == hash0) ++r;

    Move mv = m_undos[i].mv;
    if (mv == 0) break;
    if (mv.Captured()) break;
    if (mv.Piece() == PW || mv.Piece() == PB) break;
  }
  return r;
}
////////////////////////////////////////////////////////////////////////////////

bool Position::SetFEN(const string& fen)
{
  if (fen.length() < 5) return false;
  Position tmp = *this;
  Clear();

  vector<string> tokens;
  Split(fen, tokens);

  FLD f = A8;
  for (size_t i = 0; i < tokens[0].length(); ++i)
  {
    PIECE p = NOPIECE;
    switch (tokens[0][i])
    {
      case 'P': p = PW; break;
      case 'N': p = NW; break;
      case 'B': p = BW; break;
      case 'R': p = RW; break;
      case 'Q': p = QW; break;
      case 'K': p = KW; break;

      case 'p': p = PB; break;
      case 'n': p = NB; break;
      case 'b': p = BB; break;
      case 'r': p = RB; break;
      case 'q': p = QB; break;
      case 'k': p = KB; break;

      case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8':
        f += tokens[0][i] - '0';
        break;

      case '/':
        if (Col(f) != 0) f = 8 * (Row(f) + 1);
        break;

      default:
        goto ILLEGAL_FEN;
    }

    if (p)
    {
      Put(f, p);
      if (p == KW)
        m_Kings[WHITE] = f;
      else if (p == KB)
        m_Kings[BLACK] = f;
      ++f;
    }
  }

  if (tokens.size() < 2) goto FINAL_CHECK;

  if (tokens[1] == "w")
    m_side = WHITE;
  else if (tokens[1] == "b")
    m_side = BLACK;
  else
    goto ILLEGAL_FEN;

  if (tokens.size() < 3) goto FINAL_CHECK;

  for (size_t i = 0; i < tokens[2].length(); ++i)
  {
    switch (tokens[2][i])
    {
      case 'K': m_castlings |= WHITE_O_O;   break;
      case 'Q': m_castlings |= WHITE_O_O_O; break;
      case 'k': m_castlings |= BLACK_O_O;   break;
      case 'q': m_castlings |= BLACK_O_O_O; break;
      case '-': break;
      default: goto ILLEGAL_FEN;
    }
  }

  if (tokens.size() < 4) goto FINAL_CHECK;

  if (tokens[3] != "-")
  {
    m_ep = StrToFld(tokens[3]);
    if (m_ep == NF) goto ILLEGAL_FEN;
  }

FINAL_CHECK:

  return true;

ILLEGAL_FEN:

  *this = tmp;
  return false;
}
////////////////////////////////////////////////////////////////////////////////

void Position::SetInitial()
{
  SetFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}
////////////////////////////////////////////////////////////////////////////////

void Position::UnmakeMove()
{
  if (m_undoSize <= 0) return;

  COLOR opp = m_side;
  COLOR side = opp ^ 1;

  Undo& undo = m_undos[--m_undoSize];
  m_castlings = undo.castlings;
  m_ep = undo.ep;
  m_fifty = undo.fifty;
  Move mv = undo.mv;

  FLD from = mv.From();
  FLD to = mv.To();
  PIECE piece = mv.Piece();
  PIECE captured = mv.Captured();

  Remove(to);
  if (captured)
  {
    if (to == m_ep)
      Put(to + 8 - 16 * side, captured);
    else
      Put(to, captured);
  }
  Put(from, piece);

  switch (piece)
  {
    case KW:
      m_Kings[WHITE] = from;
      if (from == E1)
      {
        if (to == G1)
        {
          Remove(F1);
          Put(H1, RW);
        }
        else if (to == C1)
        {
          Remove(D1);
          Put(A1, RW);
        }
      }
      break;
    case KB:
      m_Kings[BLACK] = from;
      if (from == E8)
      {
        if (to == G8)
        {
          Remove(F8);
          Put(H8, RB);
        }
        else if (to == C8)
        {
          Remove(D8);
          Put(A8, RB);
        }
      }
      break;
    default:
      break;
  }

  --m_ply;
  m_side ^= 1;
}
////////////////////////////////////////////////////////////////////////////////

void Position::UnmakeNullMove()
{
  if (m_undoSize <= 0) return;

  Undo& undo = m_undos[--m_undoSize];
  m_castlings = undo.castlings;
  m_ep = undo.ep;
  m_fifty = undo.fifty;

  --m_ply;
  m_side ^= 1;
}
////////////////////////////////////////////////////////////////////////////////

