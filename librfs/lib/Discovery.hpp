#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>

#include "Discovery.pb.h"

#include "utils/Log.hpp"

namespace rfs
{

class Discovery : public boost::enable_shared_from_this<Discovery>
{
public:
    class Peer
    {
    public:
        static bool isValid ( const proto::Peer& peer );

        Peer();
        Peer ( const proto::Peer& peer );

        bool operator== ( const Peer& rhs ) const;
        inline bool operator!= ( const Peer& rhs ) const
        {
            return ( ! operator== ( rhs ) );
        }

        boost::uuids::uuid id;

        std::vector<boost::asio::ip::address> addrs;
        uint16_t port;
    };

    class Callback
    {
    public:
        virtual ~Callback() {}

        virtual void onPeerAdded ( const Discovery& instance, const Peer& peer ) = 0;

        virtual void onPeerUpdated ( const Discovery& instance, const Peer& peer ) = 0;

        virtual void onPeerRemoved ( const Discovery& instance, const boost::uuids::uuid& id ) = 0;
    };

    Discovery ( boost::asio::io_service& svc, Callback& cb, const boost::uuids::uuid& id, const uint16_t port );

    void start();

    void stop();

    inline uint16_t getPort() const
    {
        return port_;
    }

    inline const boost::uuids::uuid& getId() const
    {
        return id_;
    }

private:
    struct PeerEntry
    {
        Peer peer;

        time_t lastSeen;
    };

    void doReceive ( const boost::system::error_code& err, size_t bytes );
    void doPeerTimeoutCheck ( const boost::system::error_code& err );
    void doNetworkUpdate ( const boost::system::error_code& err );
    void onSendComplete ( const boost::system::error_code& err );

    static boost::asio::ip::address ListenAddr;
    static boost::asio::ip::address MulticastAddr;
    static uint16_t MulticastPort;

    static uint16_t MaxMessageSize;
    static uint16_t MaxPeerTimeoutTime;

    static Logger log_;

    Callback& cb_;

    const boost::uuids::uuid id_;
    const uint16_t port_;

    boost::asio::ip::udp::socket socket_;

    boost::asio::deadline_timer peerTimeoutCheck_;

    boost::asio::deadline_timer updateTimer_;

    boost::asio::ip::udp::endpoint lastReceivedEndpoint_;

    boost::asio::ip::udp::endpoint sendEndpoint_;

    std::vector<char> recvData_;

    std::vector<char> sendData_;

    std::unordered_map<boost::uuids::uuid, PeerEntry, boost::hash<boost::uuids::uuid> > peers_;
};

typedef boost::shared_ptr<Discovery> DiscoveryPtr;

}

