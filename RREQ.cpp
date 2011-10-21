
#include <sstream>
#include <iostream>
#include <string>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "RREQ.h"

RREQ::RREQ(std::string inType, int inSource, int inDest) :
    Message(inType),
    source(inSource), dest(inDest)
{}

std::string RREQ::toString()
{
    std::ostringstream oss;
    oss << type << " " << source << " " << dest;

    return oss.str();
}

void RREQ::fromString(string str)
{
    const char *con_str = str.c_str();
    char *myStr = new char[str.length()];
    memcpy(myStr, con_str, str.length());

    char *token = strtok(myStr, " ");
    type = token;
    token = strtok(NULL, " ");
    source = boost::lexical_cast<int>(token);
    token = strtok(NULL, " ");
    dest = boost::lexical_cast<int>(token);
}

