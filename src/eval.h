#ifndef EVAL_H
#define EVAL_H

#include "position.h"

const EVAL INFINITY_SCORE  = 50000;
const EVAL CHECKMATE_SCORE = 32767;
const EVAL DRAW_SCORE      = 0;

extern EVAL VAL_P;
extern EVAL VAL_N;
extern EVAL VAL_B;
extern EVAL VAL_R;
extern EVAL VAL_Q;

extern EVAL VALUE[14];

void InitEval();
EVAL Evaluate(const Position& pos);
EVAL Evaluate(const Position& pos, EVAL alpha, EVAL beta);

#endif

