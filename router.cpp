#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <signal.h>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "PCEconfig.h"
#include "Message.h"
#include "utils.h"
#include "usage.h"
#include "Socket.h"
#include "utils.h"

using namespace std;
using namespace cs6390;

void ui_help(string message="")
{
    if(message != "")
        cerr << message << endl << endl;
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

    RouterStatus localStatus(argc, argv);
    PCEconfig pConfig(argv[3]);

    cout << pConfig;
    cout << "localStatus (before serialization): " << endl;

    cout << localStatus;

    AS myAS=pConfig.getAS(localStatus);

    {
        cout << "Sending initial status... " << flush;
        Socket s(&myAS.saddr);
        s.sendMessage(localStatus);
        cout << "done" << endl;
    }

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
        string line;
        uint32_t arg;
        cout << "> ";
       
        getline(cin, line);
        vector<string> v;
        boost::split(v, line, boost::is_any_of(" \t"), boost::algorithm::token_compress_on );
    
        //ignore extra args...
        if(v[0] == "RT") {
            try {
                arg = boost::lexical_cast<uint32_t>(v[1]);
            }
            catch ( ... ) {
                ui_help("argument to RT must be integer");
                continue;
            }
            if (arg > 99) {
                ui_help("argument to RT must be in range [0:99]");
                continue;
            }
            
            cout<< "Calculating route to " << arg
                << " (Not implemented yet)" << endl;
        }
        else if(v[0] == "DN" || v[0] == "UP") {
            try {
                arg = boost::lexical_cast<uint32_t>(v[1]);
            }
            catch ( ... ) {
                ui_help("argument to DN must be integer");
                continue;
            }
            int rc;
            if (v[0] == "DN") {
                cout << "Bring down interface " << arg << "..." << flush;
                rc = localStatus.setLinkMetric(arg, 99);
            }
            else {
                cout << "Bring up interface " << arg << "..." << flush;
                rc = localStatus.setLinkMetric(arg, 1);
            }
            if(rc == 0) {
                Socket s(&myAS.saddr);
                s.sendMessage(localStatus);
                cout << "done" << endl;
            }
            else {
                ui_help( string("Selected network ") + v[1] + " is not a valid local network");
                continue;
            }
        }
        else if(v[0] == "LI") {
            cout << localStatus;
        }
        else if(v[0] == "") {
            continue;
        }
        else{
            ui_help( string("Unknown command: ") + v[0]);
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
