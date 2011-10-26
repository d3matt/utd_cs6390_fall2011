
#include <sstream>
#include <iostream>
#include <string>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "RREQ.h"

std::string RREQ::toString()
{
    std::ostringstream oss;
    oss << type << " " << source << " " << dest;

    return oss.str();
}

void RREQ::fromString(string str)
{
    const char *con_str = str.c_str();
    char *myStr = new char[strlen(con_str)];
    strcpy(myStr, con_str);

    try
    {
        char *token = strtok(myStr, " ");
        if(strncmp(token, "RREQ", 4) != 0)
        {
            std::cout << "BAD " << token << std::endl;
            std::cout.flush();
            throw;
        }
        token = strtok(NULL, " ");
        source = boost::lexical_cast<int>(token);
        token = strtok(NULL, " ");
        dest = boost::lexical_cast<int>(token);
    }
    catch (...)
    {
        std::cerr << "Couldn't get message from string" << std::endl;
    }

}

