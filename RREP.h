
#ifndef _RREP_H
#define _RREP_H

#include "Message.h"
#include "Path.h"

namespace cs6390
{

class RREP : public Message
{
private:
    Path p;

public:
    RREP(Path inP) : 
        Message("RREP"), p(inP)
    {}

    RREP(string construct) : Message("RREP")
    {
        fromString(construct);
    }


    virtual string toString();
    virtual void fromString(string construct);
    string serialize();
};

} //cs6390

#endif /* _RREP_H */
