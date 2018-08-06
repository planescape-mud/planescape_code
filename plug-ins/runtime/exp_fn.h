#ifndef EXP_FN_H
#define EXP_FN_H

#include "expr.h"

extern EXP_FN_STR expFnsImpl[];
void ExecExpFunctionImpl(EXP_FN_STR * FnStr, EXP_ITEM & item, EXPFN_PARAMS, EXP_ITEM ** Par);

void expfn_init();
void expfn_destroy();

#endif
