#include <netinet/in.h>
#include <netdb.h>

#include <string>

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

    AS(uint32_t ASno, std::string hostname, uint16_t portno, bool lookup=true);
};

#endif // __AS_H__
