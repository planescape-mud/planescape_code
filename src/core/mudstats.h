/* $Id$
 *
 * ruffina, 2009
 */
#ifndef MUDSTATS_H
#define MUDSTATS_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlshort.h"
#include "xmllong.h"
#include "xmlconfigurable.h"
#include "oneallocate.h"

class MudStats : public XMLConfigurable, 
                 public XMLVariableContainer,
                 public OneAllocate
{
friend class PlaneScape;
XML_OBJECT
public:
        
    MudStats();
    virtual ~MudStats();

    bool load();
    bool save();
    void update();
    
    long long genID();

    inline int getHigh() const;
    inline int getPeak() const;
    inline int getRebootLevel() const;
    inline void setRebootLevel(int level);

protected:
    virtual DLString getConfigDirPath() const;

    XML_VARIABLE XMLInteger peak;
    XML_VARIABLE XMLInteger high;
    XML_VARIABLE XMLInteger players;
    XML_VARIABLE XMLShort   rebootLevel;
    XML_VARIABLE XMLLong bootVersion;

    long long lastID;

    time_t time_high;
    time_t time_peak;
};

inline int MudStats::getHigh() const
{
    return high.getValue();
}

inline int MudStats::getPeak() const
{
    return peak.getValue();
}
inline int MudStats::getRebootLevel() const
{
    return rebootLevel.getValue();
}

inline void MudStats::setRebootLevel(int level)
{
    rebootLevel = level;
}


#endif
