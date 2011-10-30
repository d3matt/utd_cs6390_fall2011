#include "AS.h"

#include <string.h>
#include <sstream>

using namespace std;

AS::AS(uint32_t ASno, string hostname, uint16_t portno, bool lookup):
    ASno(ASno),
    hostname(hostname),
    portno(portno)
{
    if(ASno > 9) {
        throw ASexception("invalid AS number");
    }

    if(lookup) {
        struct hostent * he;
        he = gethostbyname( hostname.c_str() );
        if ( he == NULL ) {
            stringstream s;
            s << "Unabled to lookup host '" << hostname << "'";
            throw ASexception(s.str());
        }
        saddr_in.sin_family = AF_INET;
        memcpy(&saddr_in.sin_addr, he->h_addr_list[0], 4);
        saddr_in.sin_port = htons(portno);
    }
}
