#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <climits>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "PCEconfig.h"
#include "Message.h"
#include "usage.h"
#include "Socket.h"

using namespace std;
using namespace cs6390;

void ui_help()
{
    cout<< "Enter one of the following commands: " << endl
        << " RT <net>" << endl
        << " UP <net>" << endl
        << " DN <net>" << endl
        << " LI" << endl;
}

bool stopping=false;

void signal_death( int sig, siginfo_t *info, void *ctxt )
{
    cout << "Shutting down" << endl;
    stopping=true;
    close(0); //close stdin so getline() will return
}

int main(int argc, char ** argv)
{
    if(argc == 1)
        router_usage(NULL, true, 0);
    else if(argc < 7)
        router_usage("invalid number of arguments");

    LSA localStatus(argc, argv);
    PCEconfig pConfig(argv[3]);

    cout << pConfig;
    cout << "localStatus (before serialization): " << endl;

    cout << &localStatus;

    AS myAS=pConfig.getAS(localStatus);


    Socket * s = NULL;
    for (uint32_t i=0; i != 1 ;) {
        cout << "Sending initial status... " << flush;
        try
        {
            s = new Socket(&myAS.saddr);
        }
        catch( Socket::SocketException &e)
        {
            cout << " (timeout)" << endl;
            sleep(1);
            continue;
        }
        i=1;
    }

    s->sendMessage(localStatus);
    cout << "done" << endl;
    delete s;

    {
        struct sigaction action;
        action.sa_flags     = SA_SIGINFO | SA_RESTART;
        sigemptyset( &action.sa_mask );

        action.sa_sigaction = signal_death;
        sigaction( SIGHUP, &action, 0 );
        sigaction( SIGINT, &action, 0 );
        sigaction( SIGTERM, &action, 0 );
        sigaction( SIGQUIT, &action, 0 );
    }

    cout << endl << endl;

    ui_help();
    while (!stopping) {
        string line = "";
        uint32_t arg = 0;
        cout << ">>> "; //steal the python shell prompt because I love it so --Matt

        getline(cin, line);
        if(cin.eof()) break;    //die on CTRL-D

        vector<string> v;
        boost::split(v, line, boost::is_any_of(" \t"), boost::algorithm::token_compress_on );

        boost::to_upper(v[0]);
    
        //ignore extra args...
        if(v[0] == "RT") {
            try {
                arg = boost::lexical_cast<uint32_t>(v[1]);
            }
            catch ( ... ) {
                cout << "argument to RT must be integer" << endl;
                continue;
            }
            if (arg > 99) {
                cout << "argument to RT must be in range [0:99]" << endl;
                continue;
            }
            cout<< "Calculating route to " << arg << endl;

            Socket s(&myAS.saddr);
            RREQ mOUT;
            mOUT.source=localStatus.routerID;
            mOUT.dest=arg;
            s.sendMessage(mOUT);
            
            Message* min=s.getMessage();
            if(min != NULL ) {
                RRES *r = (RRES*)min;
                if(r->routers[0] == 99) {
                    cout << "No route to destination" << endl;
                }
                else {
                    cout << "Result: " << r << endl;
                }
                delete min;
            }
            else {
                cerr << "Failed!" << endl;
            }
        }
        else if(v[0] == "DN") {
            try {
                arg = boost::lexical_cast<uint32_t>(v[1]);
            }
            catch ( ... ) {
                cout << "argument to DN must be integer" << endl;
                continue;
            }
            int rc;
            cout << "Bring down interface " << arg << "..." << flush;

            rc = localStatus.setLinkMetric(arg, 99);
            if(rc == 0) {
                Socket s(&myAS.saddr);
                s.sendMessage(localStatus);
                cout << "done" << endl;
            }
            else {
                cout << "Selected network " << v[1] << " is not a valid local network" << endl;
                continue;
            }
        }
        else if(v[0] == "UP") {
            try {
                arg = boost::lexical_cast<uint32_t>(v[1]);
            }
            catch ( ... ) {
                cout << "argument to DN must be integer" << endl;
                continue;
            }
            int rc;
            cout << "Bring up interface " << arg << "..." << flush << endl;
            rc = localStatus.setLinkMetric(arg, 1);
            if(rc == 0) {
                Socket s(&myAS.saddr);
                s.sendMessage(localStatus);
                cout << "done" << endl;
            }
            else {
                cout << "Selected network " << v[1] << " is not a valid local network" << endl;
                continue;
            }
        }
        else if(v[0] == "LI") {
            cout << &localStatus;
        }
        else if(v[0] == "EXIT") {
            break;
        }
        else if(v[0] == "HELP" || v[0] == "?") {
            ui_help();
        }
        else if(v[0] == "") {
            continue;
        }
        else {
            cout << "Unknown command: " << v[0] << endl;
            ui_help();
        }
    }

    cout << endl << "Sending down status for all our links ... " << flush;
    localStatus.setLinkMetric(99);
    {
        Socket s(&myAS.saddr);
        s.sendMessage(localStatus);
    }
    cout << "done!" << endl;

    return 0;
}
