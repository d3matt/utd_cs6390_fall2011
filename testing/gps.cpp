#include <fstream>
#include <iostream>
#include <vector>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>


/////////////////////////////////////////////////////////////
// gps coordinate
//
// illustrates serialization for a simple type
//
class gps_position
{
private:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & degrees;
        ar & minutes;
        ar & seconds;
    }
    int degrees;
    int minutes;
    float seconds;

    friend int main();
    gps_position(int d, int m, float s) : degrees(d), minutes(m), seconds(s) {}
public:
    gps_position(){};
    void dump() {
        std::cout << degrees << " " << minutes << " " << seconds << std::endl;
    }
};

int main() {
    // create and open a character archive for output
    std::ofstream ofs("filename");

    // create class instance
    const gps_position g(35, 59, 24.567f);
    const gps_position h(36, 69, 14.967f);


    // save data to archive
    {
        boost::archive::xml_oarchive oa(ofs);
        std::vector<gps_position> vIN;
        vIN.push_back(g);
        vIN.push_back(h);
        // write class instance to archive
        oa << vIN;
    	// archive and stream closed when destructors are called
    }

    // ... some time later restore the class instance to its orginal state
    std::vector<gps_position> vOUT;
    {
        // create and open an archive for input
        std::ifstream ifs("filename");
        boost::archive::xml_iarchive ia(ifs);
        // read class state from archive
        ia >> vOUT;
        // archive and stream closed when destructors are called
    }
    for(std::vector<gps_position>::iterator it = vOUT.begin(); it != vOUT.end(); it++)
        it->dump();
    return 0;
}
