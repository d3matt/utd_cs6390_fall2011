#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <string>
#include <sstream>

#include "Exceptions.h"

#ifndef __AS_H__
#define __AS_H__

class AS
{
public:
    uint32_t        ASno;
    std::string     hostname;
    uint16_t        portno;

    union {
        sockaddr_in saddr_in;
        sockaddr    saddr;
    };

    struct ASexception: public easyException {
        ASexception(std::string s): easyException(s) {}
    };

    AS():ASno(99),portno(0) {}

    //Construtor takes an AS number, hostname, portno, and whether to lookup the hostname
    AS(uint32_t ASno, std::string hostname, uint16_t portno, bool lookup=true):
    ASno(ASno),
    hostname(hostname),
    portno(portno)
    {
        if(ASno > 9) {
            throw ASexception("invalid AS number");
        }
        //lookup the hostname
        if(lookup) {
            struct hostent * he;
            he = gethostbyname( hostname.c_str() );
            if ( he == NULL ) {
                std::stringstream s;
                s << "Unabled to lookup host '" << hostname << "'";
                throw ASexception(s.str());
            }
            saddr_in.sin_family = AF_INET;
            memcpy(&saddr_in.sin_addr, he->h_addr_list[0], 4);
            saddr_in.sin_port = htons(portno);
        }
    }
};

#endif // __AS_H__
