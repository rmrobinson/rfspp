#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>

#include "Discovery.pb.h"

#include "utils/Log.hpp"

namespace rfs
{

/// @brief A class which advertises and discovers services on the network.
/// Upon starting an instance of this class, it will multicast over all available
/// interfaces the specified port and IP address the requesting service
/// can be reached at, and receive these updates from other nodes.
///
/// This takes care of deduplication of updates and removals, so that if a peer
/// hasn't advertised in a specified amount of time there will be a callback
/// indicating that the peer is no longer available.
class Discovery : public std::enable_shared_from_this<Discovery>
{
public:
    /// @brief A remote instance advertising itself.
    class Peer
    {
    public:
        /// @brief Validates the correctness of a protocol message.
        /// Ensures the ID is formatted correctly, and the addresses are also valid.
        /// @param [in] peer The peer to validate.
        /// @return true if the message is valid; false otherwise.
        static bool isValid ( const proto::Peer& peer );

        /// @brief Constructor.
        Peer();
        /// @brief Constructor.
        /// @param [in] peer The protocol message to fill this object from.
        Peer ( const proto::Peer& peer );

        /// @brief Equality operator.
        /// @param [in] rhs The object to compare against.
        /// @return true if equal (address ordering is ignored), false otherwise.
        bool operator== ( const Peer& rhs ) const;

        /// @brief Inequality operator.
        /// @param [in] rhs The object to compare against.
        /// @return Oposite of operator==().
        inline bool operator!= ( const Peer& rhs ) const
        {
            return ( ! operator== ( rhs ) );
        }

        boost::uuids::uuid id; ///< The advertised ID of this peer.

        /// @brief  List of IPs the peer is reachable on.
        std::vector<boost::asio::ip::address> addrs;
        uint16_t port; ///< The port the peer is reachable on.
    };

    /// @brief Constructor.
    /// @param [in] svc The io_service to utilize.
    /// @param [in] id The ID to advertise.
    /// @param [in] port The port to advertise.
    Discovery ( boost::asio::io_service& svc, const boost::uuids::uuid& id,
                const uint16_t port );

    /// @brief Destructor.
    ~Discovery();

    /// @brief Start advertising the service.
    void start();

    /// @brief Stop advertising the service.
    void stop();

    /// @brief Exposes the port being advertised.
    /// @return Port.
    inline uint16_t getPort() const
    {
        return port_;
    }

    /// @brief Exposes the ID being advertised.
    /// @return ID.
    inline const boost::uuids::uuid& getId() const
    {
        return id_;
    }

    /// @brief Set the handler to call when a peer is added.
    /// @param [in] cb The callback to invoke when a peer is added.
    inline void setOnPeerAddedHandler ( std::function<void(const Peer&)> cb )
    {
        onPeerAddedHandler_ = cb;
    }

    /// @brief Set the handler to call when a peer is updated.
    /// @param [in] cb The callback to invoke when a peer is updated.
    inline void setOnPeerUpdatedHandler ( std::function<void(const Peer&)> cb )
    {
        onPeerUpdatedHandler_ = cb;
    }

    /// @brief Set the handler to call when a peer is removed.
    /// @param [in] cb The callback to invoke when a peer is removed.
    inline void setOnPeerRemovedHandler (
        std::function<void(const boost::uuids::uuid&)> cb )
    {
        onPeerRemovedHandler_ = cb;
    }

private:
    /// @brief The entry in the seen peers map.
    struct PeerEntry
    {
        Peer peer; ///< The peer entry as it was last seen.

        time_t lastSeen; ///< The time at which it was last seen.
    };

    /// @brief Callback provided to async_recv_from to handle incoming packets.
    /// The data received is stored in recvData_.
    /// @param [in] err An error, if it occurred.
    /// @param [in] bytes The number of bytes read.
    void doReceive ( const boost::system::error_code& err, size_t bytes );

    /// @brief Callback provided to peerTimeout timer; checks for expired peers.
    /// @param [in] err An error, if it occurred.
    void doPeerTimeoutCheck ( const boost::system::error_code& err );

    /// @brief Callback provided to update timer, multicasts peer information out.
    /// @param [in] err An error, if it occurred.
    void doNetworkUpdate ( const boost::system::error_code& err );

    /// @brief Callback provided to async_send_to which triggers when the send finishes.
    /// @param [in] err An error, if it occurred.
    void onSendComplete ( const boost::system::error_code& err );

    /// @brief Helper function which calls async_recv_from with proper parameters.
    void setupNextReceive();

    /// @brief Configuration field; stores the address to listen on.
    static boost::asio::ip::address ListenAddr;
    /// @brief Configuration field, stores the address of the multicast group to use.
    static boost::asio::ip::address MulticastAddr;
    /// @brief Configuration field, stores the port to use.
    static uint16_t MulticastPort;

    /// @brief Configuration field, the maximum size of a message to send or receive.
    static uint16_t MaxMessageSize;
    /// @brief Configuration field, the maximum amount of time to wait after the last
    /// update message is received before a peer is considered removed.
    static uint16_t MaxPeerTimeoutTime;

    static Logger log_; ///< Log stream.

    /// @brief Callbac kto invoke when a peer is added.
    std::function<void(const Peer&)> onPeerAddedHandler_;

    /// @brief Callback to invoke when a peer is updated.
    std::function<void(const Peer&)> onPeerUpdatedHandler_;

    /// @brief Callback to invoke when a peer is removed.
    std::function<void(const boost::uuids::uuid&)> onPeerRemovedHandler_;

    const boost::uuids::uuid id_; ///< This peer's id.
    const uint16_t port_; ///< This peer's port.

    /// @brief Socket which sends and receives are done on.
    boost::asio::ip::udp::socket socket_;

    /// @brief Timer used to expire peers not seen recently.
    boost::asio::deadline_timer peerTimeoutCheck_;

    /// @brief Timer used to send network updates.
    boost::asio::deadline_timer updateTimer_;

    /// @brief Unicast endpoint which the last packet was received from.
    boost::asio::ip::udp::endpoint lastReceivedEndpoint_;

    /// @brief Multicast endpoint which messages are sent to.
    boost::asio::ip::udp::endpoint sendEndpoint_;

    /// @brief Buffer of data which async_recv_from will fill.
    std::vector<char> recvData_;

    /// @brief Buffer of data which async_send_to will read from.
    std::vector<char> sendData_;

    /// @brief Map of peers which have been seen. Key is peer ID, value is peer.
    std::unordered_map<boost::uuids::uuid, PeerEntry,
        boost::hash<boost::uuids::uuid> > peers_;
};

/// @brief Type to use when storing a handle to a Discovery instance.
typedef std::shared_ptr<Discovery> DiscoveryPtr;

}

