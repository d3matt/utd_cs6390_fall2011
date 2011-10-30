#include <iostream>
#include <map>
#include <sstream>
#include <stdint.h>
#include <netinet/in.h>

#include "AS.h"
#include "Exceptions.h"

#ifndef __PCECONFIG_H__
#define __PCECONFIG_H__

using namespace std;

class PCEconfig
{
private:
    map<uint32_t,AS>     ASmap;
    string              filename;
public:
    PCEconfig(const char * filename);
    friend ostream& operator<< (ostream& out, const PCEconfig& c);
    
    struct PCEexception: public easyException {
        PCEexception(std::string s) : easyException(s) {}
        } ;

    AS  getAS(uint32_t ASno);
};

#endif // __PCECONFIG_H__
