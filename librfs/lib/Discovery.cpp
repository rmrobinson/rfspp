
extern "C"
{
#include <ifaddrs.h>
}

#include <cassert>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Discovery.hpp"

using namespace rfs;

boost::asio::ip::address Discovery::ListenAddr (
    boost::asio::ip::address::from_string ( "0.0.0.0" ) );
boost::asio::ip::address Discovery::MulticastAddr (
    boost::asio::ip::address::from_string ( "239.0.0.1" ) );
uint16_t Discovery::MulticastPort ( 4653 );
uint16_t Discovery::MaxMessageSize ( 1500 );
uint16_t Discovery::MaxPeerTimeoutTime ( 60 * 15 );

Logger Discovery::log_ ( "discovery" );

#define PEER_IDLE_CHECK_TIME 60
#define NETWORK_UPDATE_TIME 5

Discovery::Peer::Peer()
    : id ( boost::uuids::random_generator()() ), port ( 0 )
{
}

Discovery::Peer::Peer ( const proto::Peer& peer )
    : port ( peer.port() )
{
    boost::uuids::string_generator gen;
    id = gen ( peer.id() );

    for ( int i = 0; i < peer.addrs_size(); ++i )
    {
        addrs.push_back ( boost::asio::ip::address::from_string ( peer.addrs ( i ) ) );
    }
}

bool Discovery::Peer::operator== ( const Peer& rhs ) const
{
    if ( id != rhs.id || port != rhs.port || addrs.size() != rhs.addrs.size() )
        return false;

    for ( size_t i = 0; i < addrs.size(); ++i )
    {
        bool match = false;

        for ( size_t j = 0; j < rhs.addrs.size(); ++j )
        {
            if ( addrs.at ( i ) == rhs.addrs.at ( j ) )
            {
                match = true;
                break;
            }
        }

        if ( ! match )
            return false;
    }

    return true;
}

bool Discovery::Peer::isValid ( const proto::Peer& peer )
{
    if ( peer.port() < 0
         || peer.port() > UINT16_MAX
         || peer.addrs_size() < 1 )
    {
        return false;
    }

    return true;
}

Discovery::Discovery ( boost::asio::io_service& svc, const boost::uuids::uuid& id,
                       const uint16_t port )
    : id_ ( id ), port_ ( port ), socket_ ( svc ),
      peerTimeoutCheck_ ( svc, boost::posix_time::seconds ( PEER_IDLE_CHECK_TIME ) ),
      updateTimer_ ( svc, boost::posix_time::seconds ( NETWORK_UPDATE_TIME ) ),
      sendEndpoint_ ( MulticastAddr, MulticastPort )
{
    recvData_.reserve ( MaxMessageSize );
    sendData_.reserve ( MaxMessageSize );
}

Discovery::~Discovery()
{
    stop();
}

void Discovery::start()
{
    boost::asio::ip::udp::endpoint listener ( ListenAddr, MulticastPort );

    socket_.open ( listener.protocol() );
    socket_.set_option ( boost::asio::ip::udp::socket::reuse_address ( true ) );
    socket_.bind ( listener );

    // so we can send and receive from sockets on the same host
    socket_.set_option ( boost::asio::ip::multicast::enable_loopback ( true ) );
    socket_.set_option ( boost::asio::ip::multicast::join_group ( MulticastAddr ) );

    setupNextReceive();

    peerTimeoutCheck_.async_wait ( boost::bind ( &Discovery::doPeerTimeoutCheck,
                                       shared_from_this(),
                                       boost::asio::placeholders::error ) );
    updateTimer_.async_wait ( boost::bind ( &Discovery::doNetworkUpdate,
                                  shared_from_this(),
                                  boost::asio::placeholders::error ) );
}

void Discovery::stop()
{
    peerTimeoutCheck_.cancel();
    updateTimer_.cancel();

    socket_.cancel();
    socket_.close();
}

void Discovery::doReceive ( const boost::system::error_code& err, size_t bytes )
{
    assert ( bytes < MaxMessageSize );

    // Ensure that setupNextReceive() is called before return!

    if ( err )
    {
        log_ << Log::Crit << "Error on receive: " << err.message() << std::endl;
        setupNextReceive();
        return;
    }

    proto::Peer protoPeer;
    if ( !protoPeer.ParseFromArray ( &recvData_[0], bytes )
         || !Peer::isValid ( protoPeer ) )
    {
        log_ << Log::Err << "Unable to deserialize received Discovery message"
            << std::endl;

        setupNextReceive();
        return;
    }

    Peer peer ( protoPeer );

    if ( peer.id == id_ )
    {
        setupNextReceive();
        return;
    }

    auto it = peers_.find ( peer.id );

    if ( it == peers_.end() )
    {
        PeerEntry peerEntry;
        peerEntry.peer = peer;

        peers_.insert ( std::make_pair ( peer.id, peerEntry ) );

        if ( onPeerAddedHandler_ )
            onPeerAddedHandler_ ( peer );

        it = peers_.find ( peer.id );
    }
    else if ( peer != it->second.peer )
    {
        it->second.peer = peer;

        if ( onPeerUpdatedHandler_ )
            onPeerUpdatedHandler_ ( peer );
    }

    assert ( it != peers_.end() );
    it->second.lastSeen = time ( nullptr );

    setupNextReceive();
}

void Discovery::setupNextReceive()
{
    socket_.async_receive_from ( boost::asio::buffer ( &recvData_[0], MaxMessageSize ),
                                 lastReceivedEndpoint_,
                                 boost::bind ( &Discovery::doReceive, this,
                                     boost::asio::placeholders::error,
                                     boost::asio::placeholders::bytes_transferred ) );
}

void Discovery::doPeerTimeoutCheck ( const boost::system::error_code& )
{
    const time_t now = time ( nullptr );

    for ( auto it = peers_.begin(); it != peers_.end(); ++it )
    {
        if ( ( now - it->second.lastSeen ) > MaxPeerTimeoutTime )
        {
            if ( onPeerRemovedHandler_ )
                onPeerRemovedHandler_ ( it->first );

             peers_.erase ( it );
        }
    }

    peerTimeoutCheck_.async_wait ( boost::bind ( &Discovery::doPeerTimeoutCheck,
                                       shared_from_this(), _1 ) );
}

void Discovery::doNetworkUpdate ( const boost::system::error_code& )
{
    proto::Peer peer;
    peer.set_id ( boost::lexical_cast<std::string> ( id_ ) );
    peer.set_port ( port_ );

    struct ifaddrs *ifaddrs;

    if ( getifaddrs ( &ifaddrs ) < 0 )
    {
        log_ << Log::Crit << "Unable to get ifaddrs: " << strerror ( errno )
            << std::endl;
        assert ( false );
    }

    for ( struct ifaddrs* ifa = ifaddrs; ifa != nullptr; ifa = ifa->ifa_next )
    {
        if ( ! ifa->ifa_addr )
            continue;

        const int family = ifa->ifa_addr->sa_family;
        char name[INET6_ADDRSTRLEN];

        if ( family == AF_INET )
        {
            inet_ntop ( family, &((( struct sockaddr_in* ) ifa->ifa_addr)->sin_addr ),
                        name, INET_ADDRSTRLEN );
        }
        else if ( family == AF_INET6 )
        {
            inet_ntop ( family, &((( struct sockaddr_in6* ) ifa->ifa_addr)->sin6_addr ),
                        name, INET6_ADDRSTRLEN );
        }
        else
        {
            continue;
        }

        peer.add_addrs ( name );
    }

    const size_t size = peer.ByteSize();

    if ( size > MaxMessageSize )
    {
        log_ << Log::Crit << "Serialized message is too big: " << size
            << "; can't send" << std::endl;
        assert ( false );
    }

    if ( ! peer.SerializeToArray ( &sendData_[0], MaxMessageSize ) )
    {
        log_ << Log::Crit << "Unable to serialize to array" << std::endl;
        assert ( false );
    }

    socket_.async_send_to ( boost::asio::buffer ( &sendData_[0], size ), sendEndpoint_,
                            boost::bind ( &Discovery::onSendComplete, shared_from_this(),
                                boost::asio::placeholders::error ) );
}

void Discovery::onSendComplete ( const boost::system::error_code& err )
{
    if ( err )
    {
       log_ << Log::Crit << "Error sending: " << err.message() << std::endl;
    }

    updateTimer_.async_wait ( boost::bind ( &Discovery::doNetworkUpdate,
                                  shared_from_this(),
                                  boost::asio::placeholders::error ) );
}

