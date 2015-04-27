#ifndef UTILS_H
#define UTILS_H

#include "types.h"

int    GetTime();
bool   Is(const string& cmd, const string pattern, size_t minLen);
U32    Rand32();
U64    Rand64();
void   RandSeed(U64 seed);
double RandDouble();
double RandDouble(double from, double to);
void   Split(const string& str, vector<string>& tokens, const string& sep = " ");

#endif

