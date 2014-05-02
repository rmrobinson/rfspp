#include "LiteBriteController.hpp"

#include <cassert>
/// @todo include WiringPi header here

using namespace rfs;

LiteBriteController::LiteBriteController ( FileSystem& fs, const std::string& rPin,
    const std::string& gPin, const std::string& bPin )
    : Controller ( fs ), rPin_ ( rPin ), gPin_ ( gPin ), bPin_ ( bPin ),
        bulb_ ( *this, "/dev/liteBrite" )
{
    proto::modules::Lightbulb initState;
    proto::modules::Device* base = initState.mutable_base();
    base->set_name ( bulb_.getName() );
    base->set_type ( proto::modules::Device::Lightbulb );

    bulb_.update ( initState );
}

LiteBriteController::~LiteBriteController()
{
}

proto::RetCode LiteBriteController::set ( ProtoModule<proto::modules::Lightbulb>& bulb, const proto::modules::Lightbulb& )
{
    assert ( &bulb == &bulb_ );

    return proto::NotImplemented;
}

