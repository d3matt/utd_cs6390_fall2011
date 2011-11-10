
#ifndef _RREQ_H
#define _RREQ_H

#include "Message.h"

class RREQ : public Message
{
private:
    int source;
    int dest;

public:

    RREQ(int inSource=-1, int inDest=-1) :
        Message("RREQ"), source(inSource), dest(inDest)
    {}

    RREQ(string construct) : Message("RREQ")
    {
        fromString(construct);
    }

    virtual string toString();
    virtual void fromString(string construct);

    string serialize();

};

#endif /* _RREQ_H */
