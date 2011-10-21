#include "utils.h"

uint32_t * string_to_int(const char * str, uint32_t &ret)
{
    for(uint32_t i=0;i<strlen(str);i++)
    {
        if( !isdigit(str[i]) ) {
            ret=-1;
            return NULL;
        }
    }
    ret=strtoul(str, NULL, 10);
    return &ret;
}


//TODO: add argument for config file...
bool valid_AS(uint32_t AS)
{
    if(AS < 0 || AS > 9 )
        return false;
    return true;
}

bool valid_router(uint32_t router)
{
    if(router < 0 || router > 9)
        return false;
    return true;
}
