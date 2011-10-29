#include <iostream>
#include <fstream>
#include <sstream>
#include <netdb.h>
#include <string.h>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "utils.h"
#include "PCEconfig.h"


PCEconfig::PCEconfig(const char* filename)
: filename(filename)
{
    ifstream    infile;
    AS          tmp;
    string      line;

    infile.open(filename);
    if(!infile.is_open())
        throw PCEexception(string("Configfile missing: ") + filename);

    uint32_t lno=0;
    while (1) {
        getline(infile, line);
        lno++;
        if(!infile.good())
            break;
        vector<string> v;
        boost::split(v, line, boost::is_any_of(" \t"), boost::algorithm::token_compress_on );
        if(v.size() != 3) {
            stringstream s;
            s << "Invalid configuration (wrong number of entries) at line: " << lno;
            throw PCEexception(s.str());
        }

        try {
            tmp=AS( boost::lexical_cast<uint32_t>(v[0]), 
                    v[1], 
                    boost::lexical_cast<uint32_t>(v[2]) );
        }
        catch( AS::ASexception& e ) {
            cerr << "Bad AS at line: " << lno << endl;
            _exit(1);
        }
        catch( ... ) {
            stringstream s;
            s << "Invalid configuration (invalid entry) at line: " << lno;
            throw PCEexception(s.str());
        }

        if ( ASmap.find(tmp.ASno) != ASmap.end() ) {
            stringstream s;
            s << "Duplicate entries detected in configfile at line: " << lno;
            throw PCEexception(s.str());
        }
        struct hostent * he;
        he = gethostbyname( tmp.hostname.c_str() );
        if ( he == NULL ) {
            stringstream s;
            s << "Unabled to lookup host '" << tmp.hostname 
              << "' at line: " << lno;
            throw PCEexception(s.str());
        }
        tmp.saddr_in.sin_family = AF_INET;
        memcpy(&tmp.saddr_in.sin_addr, he->h_addr_list[0], 4);
        tmp.saddr_in.sin_port = htons(tmp.portno);
        ASmap[tmp.ASno]=tmp;
    }
    infile.close();
}

ostream& operator<< (ostream& out, const PCEconfig &c)
{
    out << "PCEconfig (" << c.filename << "): " << endl;
    for(map<uint32_t,AS>::const_iterator it=c.ASmap.begin(); it != c.ASmap.end(); it++)
    {
        out << "                       AS: " << it->second.ASno 
            << " host: " << it->second.hostname
            << " port: " << it->second.portno << endl;
    }
    return out;
}

AS PCEconfig::getAS(uint32_t ASno)
{
    map<uint32_t,AS>::iterator it = ASmap.find(ASno);
    if(it == ASmap.end()) {
        stringstream s;
        s << "ASno " << ASno << " isn't in ASmap!";
        throw PCEexception(s.str());
    }
    return it->second;
}
