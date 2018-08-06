#ifndef XENCHANT_H
#define XENCHANT_H
#include "parser.h"
#include "parser_const.h"
#include "parser_items.h"


/*****************************************************************************/
/*               */
/*****************************************************************************/

class CEnchant : public CParser {
    public:
        CEnchant();
        ~CEnchant();
        bool Initialization(void);
};


// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int ECH_NUMBER;  //ϊαώαςοχαξιε
extern int ECH_IDENTIFER; //ιξδεξτιζιλατος
extern int ECH_NAME;  //ξαϊχαξιε
extern int ECH_ALIAS;  //σιξοξιν
extern int ECH_DESCRIPTION; //οπισαξιε
extern int ECH_INCLUDE;  //σοσταχ
extern int ECH_INC_TYPE; //τιπ
extern int ECH_INC_COUNT; //λομιώεστχο
extern int ECH_OBJECT_TYPE; //πςεδνετ
extern int ECH_WEAR_TYPE; //ισπομψϊοχαξιε
extern int ECH_APPLY;  //χμιρξιε
extern int ECH_AFFECT;  //όζζελτω
extern int ECH_SKILL;  //υνεξιε
extern int ECH_MINIMAL;  //νιξινυν
extern int ECH_MAXIMUM;  //ναλσινυν
extern int ECH_ARM_CLASS; //λμασσ
extern int ECH_WPN_TYPE; //οςυφιε

extern CEnchant xEnchant;

#endif
