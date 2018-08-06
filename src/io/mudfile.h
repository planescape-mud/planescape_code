/* $Id$
 *
 * ruffina, 2009
 */
#ifndef MUDFILE_H
#define MUDFILE_H

#include "dlfile.h"

struct MudFile : public DLFile {
    MudFile();
    MudFile(const DLString &dirName, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString);
    MudFile(const DLFile &dir, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString);
    virtual ~MudFile();
};

struct ShareFile : public DLFile {
    ShareFile(const DLString &dirName, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString);
    ShareFile(const DLFile &dir, const DLString &filename = DLString::emptyString, const DLString &fileext = DLString::emptyString);
    virtual ~ShareFile();
};

#endif
