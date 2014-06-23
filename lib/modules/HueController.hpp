#pragma once

#include <string>
#include <vector>

#include "Controller.hpp"
#include "ProtoProcessFile.hpp"
#include "Device.pb.h"

namespace rfs
{

class HueController : public Controller<ProtoProcessFile<proto::modules::Lightbulb>, proto::modules::Lightbulb>
{
public:
    HueController ( ProcessFileSystem& fs, const std::string& url );
    ~HueController();

    virtual RetCode set ( ProtoProcessFile<proto::modules::Lightbulb>& bulb, const proto::modules::Lightbulb& state );

private:
    class CurlWrapper* curlm_;

    std::vector<ProtoProcessFile<proto::modules::Lightbulb>*> bulbs_;
};

}

