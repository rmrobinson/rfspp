#pragma once

#include "Session.hpp"

namespace rfs
{

class LocalSession : public Session
{
public:
    LocalSocket ( boost::asio::io_service& svc );
};

}

