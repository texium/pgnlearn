#include "utils.h"

U64 g_rand64 = 1;

void RandSeed(U64 seed)
{
  g_rand64 = seed;
}
////////////////////////////////////////////////////////////////////////////////


U32 Rand32()
{
  return U32(Rand64() >> 32);
}
////////////////////////////////////////////////////////////////////////////////


U64 Rand64()
{
  const U64 a = LL(2862933555777941757);
  const U64 b = LL(3037000493);

  g_rand64 = a * g_rand64 + b;
  return g_rand64;
}
////////////////////////////////////////////////////////////////////////////////

double RandDouble()
{
  return double(Rand32()) / 0xffffffff;
}
////////////////////////////////////////////////////////////////////////////////

double RandDouble(double from, double to)
{
  return from + (to - from) * RandDouble();
}
////////////////////////////////////////////////////////////////////////////////

void Split(const string& str, vector<string>& tokens, const string& sep)
{
  size_t begin, end;

  tokens.clear();
  if (str.empty()) return;

  begin = str.find_first_not_of(sep, 0);
  if (begin == string::npos) return;
  end = str.find_first_of(sep, begin);

  while (end != string::npos)
  {
    tokens.push_back(str.substr(begin, end - begin));

    begin = str.find_first_not_of(sep, end);
    if (begin == string::npos) return;
    end = str.find_first_of(sep, begin);
  }
  tokens.push_back(str.substr(begin, str.length() - begin));
}
////////////////////////////////////////////////////////////////////////////////

