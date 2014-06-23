#pragma once

#include <string>
#include <vector>

#include "Controller.hpp"
#include "ProtoProcessFile.hpp"
#include "Device.pb.h"

namespace rfs
{

class X10Controller : public Controller<ProtoProcessFile<proto::modules::Device>, proto::modules::Device>
{
public:
    X10Controller ( ProcessFileSystem& fs, const std::string& portName );
    ~X10Controller();

    virtual RetCode set ( ProtoProcessFile<proto::modules::Device>& dev, const proto::modules::Device& state );

private:
    static bool nameToId ( const std::string& devName, uint8_t& id );

    int fd_; ///< The file descriptor of the serial port to use

    std::vector<ProtoProcessFile<proto::modules::Device>*> devices_; ///< The device modules.
};

}

