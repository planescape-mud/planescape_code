#ifndef XSAVE_H
#define XSAVE_H
#include "parser.h"
#include "parser_const.h"


/*****************************************************************************/
/* ןÐÒÅÄÅÌÅÎÉÑ ÄÌÑ ÞÔÅÎÉÑ/ÚÁÐÉÓÉ ÆÁÊÌÏ× ÓÏÓÔÏÑÎÉÑ ÒÅÎÔÙ (savedata)           */
/*****************************************************************************/

class CRent : public CParser {
    public:
        CRent();
        ~CRent();
        bool Initialization(void);
};

extern int RENT_NUMBER;
extern int RENT_NAME;
extern int RENT_TYPE;
extern int RENT_TIME;
extern int RENT_ROOM;
extern int RENT_QUALITY;


class CPet : public CParser {
    public:
        CPet();
        ~CPet();
        bool Initialization(void);
};

extern int PET_NUMBER; //מןםוע
extern int PET_GUID; //חלןגיה
extern int PET_NAME;   //יםס
extern int PET_RACE;   //עבףב
extern int PET_TYPE; //פינ
extern int PET_SPELL; //תבכלימבמיו
extern int PET_TIME; //קעוםס
extern int PET_CLASS;
extern int PET_CLASS_TYPE; //נעןזוףףיס.כלבףף
extern int PET_CLASS_LEVEL; //נעןזוףףיס.ץעןקומר
extern int PET_HIT;  //פציתמר
extern int PET_MOVE;  //פגןהעןףפר
extern int PET_MANA;  //פםבמב
extern int PET_EXP;  //ןנשפ
extern int PET_LIKES;  //עבגןפב
extern int PET_SKILLS;  //ץםומיס

class CVarSave : public CParser {
    public:
        CVarSave();
        ~CVarSave();
        bool Initialization(void);
};

extern int VARS_NUMBER;  //נועוםוממבס
extern int VARS_NAME;  //מבתקבמיו
extern int VARS_VALUE;  //תמב‏ומיו
extern int VARS_TIME;  //קעוםס

#endif 
