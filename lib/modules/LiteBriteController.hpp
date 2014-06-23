#pragma once

#include <string>

#include "Controller.hpp"
#include "ProtoProcessFile.hpp"
#include "Device.pb.h"

namespace rfs
{

class LiteBriteController : public Controller<ProtoProcessFile<proto::modules::Lightbulb>, proto::modules::Lightbulb>
{
public:
    LiteBriteController ( ProcessFileSystem& fs, int rPin, int gPin, int bPin );
    ~LiteBriteController();

    virtual RetCode set ( ProtoProcessFile<proto::modules::Lightbulb>& bulb, const proto::modules::Lightbulb& state );

private:
    static bool isReady_;
    const int rPin_;
    const int gPin_;
    const int bPin_;

    ProtoProcessFile<proto::modules::Lightbulb> bulb_;
};

}

