#ifndef SERIAL_H

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#define SERIAL_H
struct Message
{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar& user;
        ar& time;
        ar& m;
    }
    std::string user;
    std::string time;
    std::string m;
};

#endif // SERIAL_H
