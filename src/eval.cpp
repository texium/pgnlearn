#include "eval.h"

struct MatStruct
{
  MatStruct() : m_sum(0), m_N(0) {}

	int m_sum;
	int m_N;
};

map<int, MatStruct> g_matTable;

EVAL VAL_P = 100;
EVAL VAL_N = 320;
EVAL VAL_B = 330;
EVAL VAL_R = 500;
EVAL VAL_Q = 900;

EVAL VALUE[14] = { 0, 0, VAL_P, VAL_P, VAL_N, VAL_N, VAL_B, VAL_B, VAL_R, VAL_R, VAL_Q, VAL_Q, 0, 0 };

