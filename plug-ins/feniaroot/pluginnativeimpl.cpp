#include "pluginnativeimpl.h"

#include "sysdep.h"
#include "structs.h"
#include "../runtime/utils.h"
#include "db.h"

/*
 * Output Fenia exceptions to every immortal in the game via syslog.
 */
void fenia_wiznet(const CodeSource::Pointer &codeSource, const DLString &key, const DLString &what)
{
    ostringstream ostr;
    ostr << "Исключение при вызове сценария " << key << ": [" 
           << codeSource->getId() << "] " << codeSource->author << ": " << codeSource->name << "\n&x";

    DLString body = what;
    // libdreamland Fenia originally uses ROM-style colors.
    body.replaces("{M", "&M").replaces("{R", "&R").replaces("{G", "&G").replaces("{x", "&x");
    ostr << body;

    DLString buf = ostr.str();
    mudlog(buf.c_str(), BRF, LVL_GOD, FALSE);
}


