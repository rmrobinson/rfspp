#include "HueController.hpp"

using namespace rfs;

HueController::HueController ( FileSystem& fs, const std::string& )
    : Controller ( fs )
{
}

HueController::~HueController()
{
    for ( size_t i = 0; i < bulbs_.size(); ++i )
    {
        if ( bulbs_.at ( i ) != 0 )
        {
            delete bulbs_.at ( i );
        }
    }

    bulbs_.clear();
}

RetCode HueController::set ( ProtoModule<proto::modules::Lightbulb>&, const proto::modules::Lightbulb& )
{
    return NotImplemented;
}

