/* $Id$
 *
 * ruffina, 2009
 */
#ifndef MUDFILEREADER_H
#define MUDFILEREADER_H

#include "dlfileloader.h"

class MudFileReader : public DLFileReaderByIndex {
public:
    MudFileReader(const DLString &dirPath);

protected:
    virtual DLDirectory getTableDir() const;
    virtual DLString getIndexName() const;

    DLDirectory tableDir;
};


#endif
