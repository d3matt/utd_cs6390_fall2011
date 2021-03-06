#include <iostream>
#include <fstream>
#include <sstream>
#include <netdb.h>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "PCEconfig.h"

namespace cs6390
{

//constructor to read a file a store the config data
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
        //splits a line on whitespace
        boost::split(v, line, boost::is_any_of(" \t"), boost::algorithm::token_compress_on );
        if(v.size() != 3) {
            stringstream s;
            s << "Invalid configuration (wrong number of entries) at line: " << lno;
            throw PCEexception(s.str());
        }

        try {
            //Make a new AS
            tmp=AS( boost::lexical_cast<uint32_t>(v[0]), 
                    v[1], 
                    boost::lexical_cast<uint32_t>(v[2]) );
        }
        catch( AS::ASexception& e ) {
            //It was bad, print errors
            stringstream s;
            s << "Bad AS (" << e.s << ") at line: " << lno << endl;
            throw PCEexception(s.str());
        }
        catch( ... ) {
            stringstream s;
            s << "Invalid configuration (invalid entry) at line: " << lno;
            throw PCEexception(s.str());
        }

        if ( ASmap.find(tmp.ASno) != ASmap.end() ) {
            //We already have this AS configured
            stringstream s;
            s << "Duplicate entries detected in configfile at line: " << lno;
            throw PCEexception(s.str());
        }
        ASmap[tmp.ASno]=tmp;
    }
    infile.close();
}

//Fucntion to print out PCEconfig
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

//Return AS object based on ASno
AS PCEconfig::getAS(const uint32_t ASno)
{
    map<uint32_t,AS>::iterator it = ASmap.find(ASno);
    if(it == ASmap.end()) {
        stringstream s;
        s << "ASno " << ASno << " isn't in ASmap!";
        throw PCEexception(s.str());
    }
    return it->second;
}

} //cs6390
