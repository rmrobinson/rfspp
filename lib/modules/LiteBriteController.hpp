#pragma once

#include <string>

#include "Controller.hpp"
#include "ProtoModule.hpp"
#include "Device.pb.h"

namespace rfs
{

class LiteBriteController : public Controller<ProtoModule<proto::modules::Lightbulb>, proto::modules::Lightbulb>
{
public:
    LiteBriteController ( FileSystem& fs, const std::string& rPin, const std::string& gPin, const std::string& bPin );
    ~LiteBriteController();

    virtual proto::RetCode set ( ProtoModule<proto::modules::Lightbulb>& bulb, const proto::modules::Lightbulb& state );

private:
    const std::string rPin_;
    const std::string gPin_;
    const std::string bPin_;

    ProtoModule<proto::modules::Lightbulb> bulb_;
};

}

