#ifndef __ROUTER_STATUS_H__
#define __ROUTER_STATUS_H__

#include <stdint.h>

#include <map>
#include <iostream>

#include "Message.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>

class Link {
public:
    uint32_t    net;
    bool        state;
    //for serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & net & state;
    }
};
ostream& operator<< (ostream& out, Link l);
//this get's around something that was fixed in newer versions of boost
//BOOST_CLASS_TRACKING(Link, boost::serialization::track_never);

typedef         std::map<int, Link> LinkMap;

class RouterStatus : public Message
{
private:
    uint32_t        AS;
    uint32_t        routerID;
    uint32_t        neighborAS;
    uint32_t        neighborrouterID;

    //used a map to make lookups fast
    LinkMap         linkStates;

    //for serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & AS & routerID & neighborAS & neighborrouterID;
        ar & linkStates;
    }

public:
    friend ostream& operator<< (ostream& out, const RouterStatus& c);

                    RouterStatus();
                    RouterStatus(int argc, char ** argv);
    int             addLink(uint32_t net);
    int             setLinkState(uint32_t net, bool state);

    uint32_t        getAS() {return AS;}
    LinkMap        *getLinkMap() {return &linkStates;}
};

#endif //__ROUTER_STATUS_H__
