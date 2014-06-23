#include "LiteBriteController.hpp"

extern "C"
{
#include <wiringPi.h>
#include <softPwm.h>
}

#include <cassert>

using namespace rfs;

LiteBriteController::isReady_ ( false );

LiteBriteController::LiteBriteController ( ProcessFileSystem& fs, int rPin, int gPin, int bPin )
    : Controller ( fs ), rPin_ ( rPin ), gPin_ ( gPin ), bPin_ ( bPin ),
        bulb_ ( *this, "/dev/liteBrite" )
{
    if ( ! isReady_ )
    {
        // This doesn't fail as of v2, so no need to check a return code.
        wiringPiSetup();
        isReady_ = true;
    }

    softPwmCreate ( rPin_, 0, 100 );
    softPwmCreate ( gPin_, 0, 100 );
    softPwmCreate ( bPin_, 0, 100 );

    proto::modules::Lightbulb initState;
    proto::modules::Device* base = initState.mutable_base();
    base->set_name ( bulb_.getName() );
    base->set_type ( proto::modules::Device::Lightbulb );

    bulb_.update ( initState );
}

LiteBriteController::~LiteBriteController()
{
}

RetCode LiteBriteController::set ( ProtoProcessFile<proto::modules::Lightbulb>& bulb, const proto::modules::Lightbulb& newState )
{
    assert ( &bulb == &bulb_ );

    if ( ! newState.has_colour() )
    {
        return proto::InvalidData;
    }

    int r = newState.colour().r();
    int g = newState.colour().g();
    int b = newState.colour().b();

    if ( r < 0 )
        r = 0;
    if ( g < 0 )
        g = 0;
    if ( b < 0 )
        b = 0;

    if ( r > 100 )
        r = 100;
    if ( g > 100 )
        g = 100;
    if ( b > 100 )
        b = 100;

    if ( newState.has_brightness() )
    {
        int brightness = newState.brightness();

        if ( brightness < 0 )
            brightness = 0;
        if ( brightness > 100 )
            brightness = 100;

        r *= brightness;
        g *= brightness;
        b *= brightness;
    }

    softPwmWrite ( rPin_, r );
    softPwmWrite ( gPin_, g );
    softPwmWrite ( bPin_, b );

    return proto::Success;
}

