
#ifndef _RREP_H
#define _RREP_H

#include "Message.h"
#include "Path.h"

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

};

#endif /* _RREP_H */
