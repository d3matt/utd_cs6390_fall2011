
#include <iostream>
#include <sstream>

#include "RREQ.h"
#include "RREP.h"

#include "Socket.h"

int main()
{

    Socket sock("localhost", 12543);

    Message *msg;

    msg = new RREQ(4, 5);
    
    sock << msg;

    delete msg;
    msg = NULL;

    std::istringstream ss(sock.receiveFromSocket());

    std::string type;
    ss >> type;

    if(type == "RREQ")
    {
        msg = new RREQ(ss.str());
    }
    else if(type == "RREP")
    {
        msg = new RREP(ss.str());
    }


    if(msg != NULL)
    {
        std::cout << msg << std::endl;
    }

    return 0;
}
