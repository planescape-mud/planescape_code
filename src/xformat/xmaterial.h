#ifndef XMATERIAL_H
#define XMATERIAL_H
#include "parser.h"
#include "parser_const.h"

/*****************************************************************************/
/* Определения для загрузки файла материала                                  */
/*****************************************************************************/

class CMaterial : public CParser {
    public:
        CMaterial();
        ~CMaterial();
        bool Initialization(void);
};

// Определение основных параметром
extern int MAT_NUMBER; //МАТЕРИАЛ (ключ)
extern int MAT_DESCRIPTION;
extern int MAT_NAME; //МАТЕРИАЛ (название)
extern int MAT_WEIGHT; //ВЕС
extern int MAT_COST; //ЦЕНА
extern int MAT_DURAB; //ПРОЧНОСТЬ
extern int MAT_AC; //БРОНЯ
extern int MAT_ARM0; //РЕЖУЩЕЕ
extern int MAT_ARM1; //КОЛЮЩЕЕ
extern int MAT_ARM2; //УДАРНОЕ
extern int MAT_HITX;
extern int MAT_HITY;
extern int MAT_HITZ;
extern int MAT_ALIAS;
extern int MAT_TYPE;
extern int MAT_HITS;
extern int MAT_FIRE;
extern int MAT_COLD;
extern int MAT_ELECTRO;
extern int MAT_ACID;
extern int MAT_INCLUDE; //КОМПОНЕНТ

#endif
