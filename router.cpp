#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

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
        MessageContainer m(&localStatus);
        s.sendMessage(m);
        cout << "done" << endl;
    }

    cout << endl << endl;
    ui_help();
    while (1) {
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
                rc = localStatus.setLinkState(arg, false);
            }
            else {
                cout << "Bring up interface " << arg << "..." << flush;
                rc = localStatus.setLinkState(arg, true);
            }
            if(rc == 0) {
                Socket s(&myAS.saddr);
                MessageContainer m(&localStatus);
                s.sendMessage(m);
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

    return 0;
}
