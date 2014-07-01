#include "Session.hpp"

using namespace rfs;

LocalSession::LocalSession ( boost::asio::io_service& svc, const std::string& file )
    : socket_ ( boost::asio::local::stream_protocol::socket ( svc ) )
{
}

