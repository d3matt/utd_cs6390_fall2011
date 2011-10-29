#include <iostream>
#include <map>
#include <sstream>
#include <stdint.h>
#include <netinet/in.h>



using namespace std;

class AS
{
public:
    uint32_t        ASno;
    string          hostname;
    uint16_t        portno;

    union {
        sockaddr_in saddr_in;
        sockaddr    saddr;
    };

    class ASexception: public exception { };

    AS():ASno(99),portno(0) {}

    AS(uint32_t ASno, string hostname, uint16_t portno) :
        ASno(ASno),
        hostname(hostname),
        portno(portno)
    {
        if(ASno > 9) {
            throw ASexception();
        }
//        if(portno > 0xffff) {
//          throw ASexception();
//        }
    }
};

class PCEconfig
{
private:
    map<uint32_t,AS>     ASmap;
    string              filename;
public:
    PCEconfig(const char * filename);
    friend ostream& operator<< (ostream& out, const PCEconfig& c);
    
    class PCEexception: public exception
    {
        string s;
        virtual const char* what() const throw()
        {
            return s.c_str();
        }
    public:
        PCEexception(string s) : s(s) {}
        ~PCEexception() throw() {}
    };

    AS  getAS(uint32_t ASno);
};
