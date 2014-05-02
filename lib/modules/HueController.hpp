#pragma once

#include <string>
#include <vector>

#include "Controller.hpp"
#include "ProtoModule.hpp"
#include "Device.pb.h"

namespace rfs
{

class HueController : public Controller<ProtoModule<proto::modules::Lightbulb>, proto::modules::Lightbulb>
{
public:
    HueController ( FileSystem& fs, const std::string& url );
    ~HueController();

    virtual proto::RetCode set ( ProtoModule<proto::modules::Lightbulb>& bulb, const proto::modules::Lightbulb& state );

private:
    class CurlWrapper* curlm_;

    std::vector<ProtoModule<proto::modules::Lightbulb>*> bulbs_;
};

}

