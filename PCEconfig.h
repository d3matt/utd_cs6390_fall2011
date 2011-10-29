#include <iostream>
#include <vector>
#include <stdint.h>

using namespace std;

class AS
{
public:
    uint8_t     ASno;
    string      hostname;
    uint16_t    portno;
};

class PCEconfig
{
private:
    vector<AS>  v;
    string      filename;
public:
    PCEconfig(const char * filename);
    friend ostream& operator<< (ostream& out, const PCEconfig& c);

    class PCEexception: public exception
    {
        virtual const char* what() const throw()
        {
            return "Configfile invalid";
        }
    };
};
