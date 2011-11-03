
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <iostream>
#include <string>
#include <vector>

#include <boost/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

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

    Message(string type) : type(type) {}
    Message() : type("XXXX") {}
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

/* 
  tracking BOOST_VERSION like this is bad, but there's not a very good way to 
compile on cs1 + our fedora15 machines...
 */
#if BOOST_VERSION == 103301
BOOST_IS_ABSTRACT(Message)
#else
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Message)
#endif

#endif
