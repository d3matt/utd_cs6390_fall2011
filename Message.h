
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

using std::string;
using std::ostream;
using std::istream;

class Message
{

protected:
    string type;

public:
    Message(string type) : type(type) {}
    Message() : type("XXXX") {}
    virtual ~Message(void) {}

    virtual string serialize(void) = 0;

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

typedef         std::map<uint32_t, uint32_t> LinkMap;

class RouterStatus : public Message
{
private:
    uint32_t        AS;
    uint32_t        routerID;
    uint32_t        neighborAS;
    uint32_t        neighborrouterID;

    //used a map to make lookups fast
    LinkMap         linkStates;

public:
    friend ostream& operator<< (ostream& out, const RouterStatus& c);
                    RouterStatus();
                    RouterStatus(int argc, char ** argv);
                    ~RouterStatus() { std::cerr << "~RouterStatus()" << std::endl; }
    int             addLink(uint32_t net, uint32_t metric=1);
    int             setLinkMetric(uint32_t net, uint32_t metric);
    int             setLinkMetric(uint32_t metric);

    uint32_t        getAS() const {return AS;}
    LinkMap        *getLinkMap() {return &linkStates;}

    //for send/recv
                    RouterStatus(std::vector<std::string> &v);
    string          serialize(void);
};



#endif
