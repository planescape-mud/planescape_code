/**************************************************************************
    МПМ "Грани Мира" (с) 2002-2003 Андрей Ермишин
    Загрузка файлов игрового мира
 **************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "xbody.h"
#include "parser_id.h"
#include "constants.h"
#include "planescape.h"
#include "mudfile.h"

///////////////////////////////////////////////////////////////////////////////
void boot_bodys(void)
{
    int numadd, i;

    if (!tBody.Initialization())
        exit(1);

    if (!tBody.ReadConfig(ShareFile(mud->bodyFile).getCPath()))
        exit(1);

    if (!tProf.Initialization())
        exit(1);

    if (!tProf.ReadConfig(ShareFile(mud->classesFile).getCPath()))
        exit(1);

    numadd = tProf.GetNumberItem();
    for (i = 0; i < numadd; i++) {
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][0] =
            tProf.GetItem(i)->GetItem(TCL_ADDSTR)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][1] =
            tProf.GetItem(i)->GetItem(TCL_ADDCON)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][2] =
            tProf.GetItem(i)->GetItem(TCL_ADDDEX)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][3] =
            tProf.GetItem(i)->GetItem(TCL_ADDINT)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][4] =
            tProf.GetItem(i)->GetItem(TCL_ADDWIS)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][5] =
            tProf.GetItem(i)->GetItem(TCL_ADDCHA)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][6] =
            tProf.GetItem(i)->GetItem(TCL_HEALTH)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][7] =
            tProf.GetItem(i)->GetItem(TCL_MANA)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][8] =
            tProf.GetItem(i)->GetItem(TCL_HROLL)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][9] =
            tProf.GetItem(i)->GetItem(TCL_AC)->GetFloat();

        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][10] =
            tProf.GetItem(i)->GetItem(TCL_ARM0)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][11] =
            tProf.GetItem(i)->GetItem(TCL_ARM1)->GetFloat();
        add_classes[tProf.GetItem(i)->GetItem(TCL_CLASS)->GetInt()][12] =
            tProf.GetItem(i)->GetItem(TCL_ARM2)->GetFloat();
    }
}
