#include <iostream>
#include <sstream>
#include <memory>

#include "RREQ.h"
#include "RREP.h"

#include "Socket.h"

using namespace std;

int main()
{

    Socket sock("localhost", 12543);

    Message *msg;

    msg = new RouterStatus();

    ((RouterStatus *)msg)->addLink(1,1);

    sock.sendMessage(*msg);
    
    delete msg;
    msg = NULL;

    msg = sock.getMessage();

    if(msg != NULL) {
        try {
            auto_ptr<RouterStatus> r(dynamic_cast<RouterStatus *> (msg));
            cout << *r << endl;
        }
        catch (...) {
            cerr << "failed to dynamic cast" << endl;
        }
    }

    return 0;
}
