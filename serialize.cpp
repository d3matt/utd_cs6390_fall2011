

/* This was adapted from the example on:
 * http://www.boost.org/doc/libs/1_47_0/libs/serialization/doc/index.html
 */

/* STD headers */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

/* Boost headers */
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/version.hpp>

/* My Headers */
#include "Socket.h"

class TestClass
{
private:
    //I'm guessing give boost::serialization access to the class...
    friend class boost::serialization::access;

    /* in this function 
     * '<<' can be replaced by '&'
     * but that feels unnatural
     */
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & myInt;
        ar & myDouble;
        ar & myString;
    }

public:
    
    TestClass(){};
    TestClass(int i, double d, std::string s) :
        myInt(i), myDouble(d), myString(s)
    {}

    int myInt;
    double myDouble;
    std::string myString;

    friend std::ostream & operator<< (std::ostream &ostr, TestClass *tst)
    {
        ostr << tst->myInt << " " << tst->myDouble << " " << tst->myString;
        return ostr;
    }

    friend void operator>> (std::string &str, TestClass *tst)
    {
        tst->myInt = 0xdeadbeef;
        tst->myDouble = -1.0;
        tst->myString = str;
    }

};

#if BOOST_VERSION == 103301
BOOST_CLASS_TRACKING(TestClass, boost::serialization::track_never);
#endif /* BOOST_VERSION */

int main(int argc, char* argv[])
{
    TestClass test(4, 8.43, "Hello World!");
/*
    Socket sock("localhost", 12544);

    sock << &test;

    TestClass inTest;

    sock >> &inTest;

    std::cout << "myInt: " << inTest.myInt << std::endl;
    std::cout << "myDouble: " << inTest.myDouble << std::endl;
    std::cout << "myString: " << inTest.myString << std::endl;
*/
    return 0;
}
    

