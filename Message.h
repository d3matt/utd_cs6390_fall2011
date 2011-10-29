
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <iostream>
#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>

using std::string;
using std::ostream;
using std::istream;

class Message
{

protected:
    string type;

    //for serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & type;
    }

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

//this get's around something that was fixed in newer versions of boost
BOOST_CLASS_TRACKING(Message, boost::serialization::track_never);

#endif
