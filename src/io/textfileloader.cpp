/* $Id$
 *
 * ruffina, 2009
 */
#include "textfileloader.h"
#include "planescape.h"

TextFileLoader::TextFileLoader()
{
    checkDuplicate(mud->getTextFileLoader());
}

DLDirectory TextFileLoader::getTableDir() const
{
    return mud->getTextDir();
}

