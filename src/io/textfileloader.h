/* $Id$
 *
 * ruffina, 2009
 */
#ifndef TEXTFILELOADER_H
#define TEXTFILELOADER_H

#include "oneallocate.h"
#include "dlfileloader.h"

class TextFileLoader : public OneAllocate, public DLFileLoader {
public:    
    TextFileLoader();

protected:
    virtual DLDirectory getTableDir() const;
};

#endif
