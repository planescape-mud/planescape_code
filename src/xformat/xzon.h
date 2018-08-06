#ifndef XZON_H
#define XZON_H
#include "parser.h"
#include "parser_const.h"

/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΟΧ ΟΠΙΣΑΞΙΡ ΪΟΞΩ (*.znx)                  */
/*****************************************************************************/

class CZon : public CParser {
    public:
        CZon();
        ~CZon();
        bool Initialization(void);
};

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int ZON_NUMBER;  // ϊοξα
extern int ZON_NAME_MAJ;  // οβμαστψ
extern int ZON_NAME_MIN;  // ξαϊχαξιε
extern int ZON_AUTHOR;  // αχτος
extern int ZON_DESCRIPTION;  // οπισαξιε
extern int ZON_TYPE;   // τιπ
extern int ZON_TOP;   // πςεδεμ
extern int ZON_RESET_TYPE;  // σβςοσ
extern int ZON_RESET_TIME;  // φιϊξψ
extern int ZON_TIME;  // χςενρ
extern int ZON_RECALL;  // ςελομμ
extern int ZON_COMMET;  // λοννεξταςικ
extern int ZON_MESTO;  // νεστοπομοφεξιε
extern int ZON_MESTO_PLANE; // νεστοπομοφεξιε.ηςαξψ
extern int ZON_MESTO_WARD; // νεστοπομοφεξιε.ςακοξ

extern CZon Zon;

#endif
