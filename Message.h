
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <iostream>
#include <string>

using std::string;
using std::ostream;
using std::istream;

class Message
{

protected:
    string type;

public:

    Message(string inType) : type(inType) {}
    virtual ~Message(void) {}

    virtual string toString(void)
    {
        return type;
    }

    virtual void fromString(string construct) {}

    friend ostream & operator<< (ostream &ostr, Message *msg)
    {
        ostr << msg->toString();
        return ostr;
    }

    friend void operator>> (string str, Message *msg)
    {
        msg->fromString(str);
    }

};

#endif
