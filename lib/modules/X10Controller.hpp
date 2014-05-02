#pragma once

#include <string>
#include <vector>

#include "Controller.hpp"
#include "ProtoModule.hpp"
#include "Device.pb.h"

namespace rfs
{

class X10Controller : public Controller<ProtoModule<proto::modules::Device>, proto::modules::Device>
{
public:
    X10Controller ( FileSystem& fs, const std::string& portName );
    ~X10Controller();

    virtual proto::RetCode set ( ProtoModule<proto::modules::Device>& dev, const proto::modules::Device& state );

private:
    static bool nameToId ( const std::string& devName, uint8_t& id );

    int fd_; ///< The file descriptor of the serial port to use

    std::vector<ProtoModule<proto::modules::Device>*> devices_; ///< The device modules.
};

}

