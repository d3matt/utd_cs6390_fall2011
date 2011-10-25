#include <iostream>
#include <vector>

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
    friend ostream& operator<< (ostream& out, PCEconfig *c);
};
