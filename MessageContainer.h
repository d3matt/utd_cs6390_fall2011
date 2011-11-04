#ifndef __MESSAGE_CONTAINER_H__
#define __MESSAGE_CONTAINER_H__

#include "Message.h"
#include "RouterStatus.h"

class MessageContainer
{
    friend class boost::serialization::access;
    Message * m;
    
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar.register_type(static_cast<RouterStatus *>(NULL));
        ar & m;
    }

public:
    MessageContainer(Message * m) : m(m) {}
    MessageContainer() { }
    Message * getMessage() { return m; }
};

//void send_MessageContainer(const MessageContainer &m, std::ostream &out);

#endif //__MESSAGE_CONTAINER_H__
