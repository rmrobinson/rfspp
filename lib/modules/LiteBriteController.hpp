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
    LiteBriteController ( FileSystem& fs, int rPin, int gPin, int bPin );
    ~LiteBriteController();

    virtual proto::RetCode set ( ProtoModule<proto::modules::Lightbulb>& bulb, const proto::modules::Lightbulb& state );

private:
    static bool isReady_;
    const int rPin_;
    const int gPin_;
    const int bPin_;

    ProtoModule<proto::modules::Lightbulb> bulb_;
};

}

