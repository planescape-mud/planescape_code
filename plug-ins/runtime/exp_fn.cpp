#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "xboot.h"
#include "db.h"
#include "exp_fn.h"
#include "parser_utils.h"

// Типы результата и параметров
// EXPTYPE_DIGIT  - число (любое. int, float, double)
// EXPTYPE_STRING - строка
// EXPTYPE_VECTOR - вектор
// EXPTYPE_LIST   - лист

//struct EXP_FN_STR
//{
// int Number;
// char Name[MAX_FN_NAME];   // 50
// EXP_TYPE TypeResult;
// int NumParam;
// EXP_TYPE TypeParam[MAX_FN_PARAM]; // 10
//};
EXP_FN_STR expFnsImpl[] = {
    {0, "rnd", EXP_TYPE(EXPTYPE_DIGIT), 2, {EXP_TYPE(EXPTYPE_DIGIT), EXP_TYPE(EXPTYPE_DIGIT),},},
    {1, "игрок.сила", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {2, "игрок.тело", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {3, "игрок.ловкость", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {4, "игрок.интелект", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {5, "игрок.мудрость", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {6, "игрок.обаяние", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {7, "игрок.уровень", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {8, "игрок.задание", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {9, "игрок.тзадание", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {10, "игрок.экипировка", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {11, "игрок.инвентарь", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {12, "игрок.наличность", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {13, "игрок.пол", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {14, "игрок.раса", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {15, "игрок.имя", EXP_TYPE(EXPTYPE_STRING), 0,},
    {16, "игрок.класс", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {17, "игрок.наклоность", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {18, "игрок.вес", EXP_TYPE(EXPTYPE_DIGIT), 0,},
    {100, "мир.переменная", EXP_TYPE(EXPTYPE_STRING), 1, {EXP_TYPE(EXPTYPE_STRING),},},
    {101, "мир.монстр", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {102, "мир.предмет", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {200, "комната.монстр", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {201, "комната.предмет", EXP_TYPE(EXPTYPE_DIGIT), 1, {EXP_TYPE(EXPTYPE_DIGIT),},},
    {300, "переменная.строка", EXP_TYPE(EXPTYPE_STRING), 0,},
    {-1,},
};

void ExecExpFunctionImpl(EXP_FN_STR * FnStr, EXP_ITEM & item, EXPFN_PARAMS, EXP_ITEM ** Par)
{
    int fnum = FnStr->Number;

    if (fnum == 0) {
        // RND
        int ret =
            rand() % (Par[1]->GetDigitInt() - Par[0]->GetDigitInt() + 1) + Par[0]->GetDigitInt();
        item.SetParam(ret);
        //debug("rnd %d", ret);
        return;
    }
    if (fnum == 1) {
        item.SetParam(GET_REAL_STR(actor));
        return;
    }

    if (fnum == 2) {
        item.SetParam(GET_REAL_CON(actor));
        return;
    }

    if (fnum == 3) {
        item.SetParam(GET_REAL_DEX(actor));
        return;
    }

    if (fnum == 4) {
        item.SetParam(GET_REAL_INT(actor));
        return;
    }

    if (fnum == 5) {
        item.SetParam(GET_REAL_WIS(actor));
        return;
    }

    if (fnum == 6) {
        item.SetParam(GET_REAL_CHA(actor));
        return;
    }

    if (fnum == 7) {
        item.SetParam(GET_LEVEL(actor));
        return;
    }

    if (fnum == 8) {
        int result = get_quested(actor, Par[0]->GetDigitInt());

        item.SetParam(result);
        return;
    }

    if (fnum == 9) {
        int result = get_current_quest(actor, Par[0]->GetDigitInt());

        item.SetParam(result);
        return;
    }

    if (fnum == 10) {
        struct obj_data *obj = GET_EQ(actor, Par[0]->GetDigitInt());
        int result = -1;

        if (obj)
            result = GET_OBJ_VNUM(obj);
        item.SetParam(result);
        return;
    }

    if (fnum == 11) {
        item.SetParam(focn(actor, Par[0]->GetDigitInt()));
        return;
    }

    if (fnum == 12) {
        item.SetParam(GET_GOLD(actor));
        return;
    }

    if (fnum == 13) {
        item.SetParam(GET_SEX(actor));
        return;
    }

    if (fnum == 14) {
        item.SetParam(GET_RACE(actor));
        return;
    }

    if (fnum == 15) {
        item.SetParam(GET_NAME(actor));
        return;
    }

    if (fnum == 16) {
        item.SetParam(actor->classes[(Par[0]->GetDigitInt())]);
        return;
    }

    if (fnum == 17) {
        item.SetParam(GET_ALIGNMENT(actor));
        return;
    }

    if (fnum == 18) {
        item.SetParam(GET_REAL_WEIGHT(actor));
        return;
    }

    if (fnum == 100) {
        item.SetParam(global_vars.check(Par[0]->GetString()));
        return;
    }

    if (fnum == 101) {
        item.SetParam(count_mob_vnum(Par[0]->GetDigitInt()));
        return;
    }

    if (fnum == 102) {
        item.SetParam(count_obj_vnum(Par[0]->GetDigitInt()));
        return;
    }

    if (fnum == 200) {
        int in_room = IN_ROOM(actor);

        if (in_room == NOWHERE) {
            item.SetParam(0);
            return;
        };
        int result = mobs_in_room(Par[0]->GetDigitInt(), in_room);

        item.SetParam(result);
        return;
    }

    if (fnum == 201) {
        int in_room = IN_ROOM(actor);

        if (in_room == NOWHERE) {
            item.SetParam(0);
            return;
        };
        int result = obj_in_room(Par[0]->GetDigitInt(), in_room);

        item.SetParam(result);
        return;
    }

    if (fnum == 300) {
        item.SetParam((const char *) arg);
        return;
    }


    ALARM;
}

void expfn_init()
{
    expFns = expFnsImpl;
    ExecExpFunction = ExecExpFunctionImpl;
    expFuncRegistry->recover();
}

void expfn_destroy()
{
    expFuncRegistry->backup();
    expFns = emptyExpFns;
    ExecExpFunction = EmptyExecExpFunction;
}

void EXP_FUNC_REGISTRY::backup()
{
    for (iterator f = begin(); f != end(); f++)
        for (int i = 0; expFnsImpl[i].Name[0] != 0; i++) {
            if ((*f)->FnName == expFns[i].Name) {
                (*f)->FnStr = NULL;
                break;
            }
        }
}

void EXP_FUNC_REGISTRY::recover()
{
    for (iterator f = begin(); f != end(); f++)
        for (int i = 0; expFnsImpl[i].Name[0] != 0; i++) {
            if ((*f)->FnName == expFns[i].Name) {
                (*f)->FnStr = &expFns[i];
                break;
            }
        }
}
