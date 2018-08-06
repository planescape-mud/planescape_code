/* $Id$
 *
 * ruffina, 2009
 */
#include "mudfile.h"
#include "planescape.h"

MudFile::MudFile()
{
}

MudFile::MudFile(const DLString &dirName, const DLString &filename, const DLString &fileext)
           : DLFile(DLDirectory(mud->getBaseDir(), dirName),
                    filename, 
                    fileext)
{
}

MudFile::MudFile(const DLFile &dir, const DLString &filename, const DLString &fileext)
          : DLFile(DLDirectory(mud->getBaseDir(), dir),
                   filename,
                   fileext)
{
}

MudFile::~MudFile()
{
}


ShareFile::ShareFile(const DLString &dirName, const DLString &filename, const DLString &fileext)
           : DLFile(DLDirectory(mud->getShareDir(), dirName),
                    filename, 
                    fileext)
{
}

ShareFile::ShareFile(const DLFile &dir, const DLString &filename, const DLString &fileext)
          : DLFile(DLDirectory(mud->getShareDir(), dir),
                   filename,
                   fileext)
{
}

ShareFile::~ShareFile()
{
}

