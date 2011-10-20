#include "utils.h"

int * string_to_int(const char * str, uint32_t &ret)
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
