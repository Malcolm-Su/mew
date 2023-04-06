#ifndef MEW_ADDRESS_H
#define MEW_ADDRESS_H

#include <string>

#include <stdio.h>

namespace mew {

class Address {
private:
    const char* ip_;
    const int   port_;

public:
    Address(const char* port = "80") 
        : ip_("0.0.0.0"), port_(atoi(port)) 
    {}

    Address(const int port = 80) 
        : ip_("0.0.0.0"), port_(port) 
    {}

    Address(const char* ip, const char* port) 
        : ip_(ip), port_(atoi(port)) 
    {}

    Address(const char* ip, const int port) 
        : ip_(ip), port_(port) 
    {}

    const char* GetIp() const { return ip_; }

    int GetPort() const { return port_; }

    std::string ToString() const {
        char buf[32];
        int len = snprintf( buf, sizeof(buf), "%s:%d", ip_, port_ );
        return std::string( buf, buf + len );
    }
};



} // namespace mew
#endif // MEW_ADDRESS_H