

/* This was adapted from the example on:
 * http://www.boost.org/doc/libs/1_47_0/libs/serialization/doc/index.html
 */

/* STD headers */
#include <iostream>
#include <fstream>
#include <string>

/* Boost headers */
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

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

};

int main(int argc, char* argv[])
{
    std::ofstream outFile("file.dat");

    TestClass test(4, 8.43, "Hello World!");

    /* Neat way of destroying the archive */
    {
        boost::archive::text_oarchive outArchive(outFile);
        outArchive << test;
    }

    TestClass inTest;

    /* Another one! */
    {
        std::ifstream inFile("file.dat");
        boost::archive::text_iarchive inArchive(inFile);

        inArchive >> inTest;
    }

    std::cout << "myInt: " << inTest.myInt << std::endl;
    std::cout << "myDouble: " << inTest.myDouble << std::endl;
    std::cout << "myString: " << inTest.myString << std::endl;

    return 0;
}
    

