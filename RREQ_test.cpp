
#include <iostream>

#include "RREQ.h"

#include "Socket.h"

#if BOOST_VERSION == 103301
BOOST_CLASS_TRACKING(TestClass, boost::serialization::track_never);
#endif /* BOOST_VERSION */

int main()
{

    Socket sock("localhost", 12544);

    Message *msg = new RREQ();
    std::string str = "RREQ 4 5";
    str >> msg;

    sock << msg;

    Message *newMsg = new RREQ();
    sock >> newMsg;

    std::cout << newMsg << std::endl;

    return 0;
}
