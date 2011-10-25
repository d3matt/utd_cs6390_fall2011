#include <iostream>
#include <fstream>
#include <sstream>

#include "PCEconfig.h"


PCEconfig::PCEconfig(const char* filename)
: filename(filename)
{
    ifstream    infile;
    AS          tmp;
    string      line;

    infile.open(filename);
    while (1) {
        getline(infile, line);
        if(!infile.good())
            break;

        stringstream str;
        str << line;
        str >> tmp.ASno;
        str >> tmp.hostname;
        str >> tmp.portno;
        v.push_back(tmp);
    }
    infile.close();
}

ostream& operator<< (ostream& out, PCEconfig *c)
{
    out << "PCEconfig (" << c-> filename << "): " << endl;
    for(vector<AS>::iterator it=c->v.begin(); it < c->v.end(); it++)
    {
        out << "                       AS: " << it->ASno 
            << " host: " << it->hostname
            << " port: " << it->portno << endl;
    }
    return out;
}
