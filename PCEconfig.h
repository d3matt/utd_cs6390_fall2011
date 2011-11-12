#include <iostream>
#include <map>
#include <sstream>
#include <stdint.h>
#include <netinet/in.h>

#include "AS.h"
#include "Exceptions.h"
#include "Message.h"

#ifndef __PCECONFIG_H__
#define __PCECONFIG_H__

using namespace std;

namespace cs6390
{

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

    AS  getAS(const uint32_t ASno);
    AS  getAS(const RouterStatus& r) { return getAS( r.getAS() ); }
};

} //cs6390

#endif // __PCECONFIG_H__
