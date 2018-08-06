/* $Id$
 *
 * ruffina, 2009
 */
#include "mudfilereader.h"
#include "planescape.h"

MudFileReader::MudFileReader(const DLString &dirName)
{
    tableDir = DLDirectory(mud->getShareDir(), dirName);
}

DLDirectory MudFileReader::getTableDir() const
{
    return tableDir;
}

DLString MudFileReader::getIndexName() const
{
    return mud->modMini ? mud->indexFileMini
                        : mud->indexFile;
}


