
#ifndef _RREQ_H
#define _RREQ_H

#include "Message.h"

class RREQ : public Message
{
private:
    int source;
    int dest;

public:

    RREQ(string inType="", int source=-1, int dest=-1);

    virtual string toString();
    virtual void fromString(string construct);

};

#endif

