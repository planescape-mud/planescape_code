#ifndef XSKILLS_H
#define XSKILLS_H
#include "parser.h"
#include "parser_const.h"
#include "parser_items.h"

/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΟΧ ΟΠΙΣΑΞΙΡ ΥΝΕΞΙΡ (spells) */
/*****************************************************************************/

class CSkl : public CParser {
    public:
        CSkl();
        ~CSkl();
        bool Initialization(void);
};


// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int SKL_NUMBER;  //υνεξιε
extern int SKL_NAME;   //ξαϊχαξιε
extern int SKL_ALIAS;  //ιδεξτιζιλατος
extern int SKL_USELESS; //ισπομψϊοχαξιε (ΣΤÒΥΛΤΥÒΑ)
extern int SKL_USE_CLASS; //πςοζεσσιρ
extern int SKL_USE_TEMPL; //τενπμεκτ
extern int SKL_USE_PRACT; //τςεξιςοχλα
extern int SKL_USE_LEVEL; //υςοχεξψ
extern int SKL_USE_EFFECT; //όζζελτιχξοστψ
extern int SKL_MISS_ATTACK; //πςοναθ (ΣΤÒΥΛΤΥÒΑ)
extern int SKL_MA_CHAR;
extern int SKL_MA_VICT;
extern int SKL_MA_ROOM;
extern int SKL_HIT_ATTACK; //ποςαφεξιε
extern int SKL_HA_CHAR;
extern int SKL_HA_VICT;
extern int SKL_HA_ROOM;
extern int SKL_DTH_ATTACK; //σνεςτψ
extern int SKL_DA_CHAR;
extern int SKL_DA_VICT;
extern int SKL_DA_ROOM;
extern int SKL_NHS_ATTACK; //ξευρϊχινοστψ
extern int SKL_NA_CHAR;
extern int SKL_NA_VICT;
extern int SKL_NA_ROOM;
extern int SKL_ARM_ATTACK; //ποημούεξιε
extern int SKL_AM_CHAR;
extern int SKL_AM_VICT;
extern int SKL_AM_ROOM;
extern int SKL_ARM2_ATTACK; //αμψτεςξατιχα
extern int SKL_A2_CHAR;
extern int SKL_A2_VICT;
extern int SKL_A2_ROOM;
extern int SKL_TMPL;
extern int SKL_TM_CHAR;
extern int SKL_TM_VICT;
extern int SKL_TM_ROOM;
extern int SKL_TMPS;
extern int SKL_TS_CHAR;
extern int SKL_TS_VICT;
extern int SKL_TS_ROOM;

extern CSkl Skl;

#endif
