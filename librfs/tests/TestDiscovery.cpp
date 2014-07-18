#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Discovery.hpp"


using namespace rfs;

class TestDiscovery
{
public:
    TestDiscovery() : d_ ( new Discovery ( svc_, boost::uuids::random_generator()(),
                          rand() % UINT16_MAX ) ) {}

    void run()
    {
        std::cout << "Starting to advertise ["
            << boost::lexical_cast<std::string> ( d_->getId() ) << "] on port "
            << std::to_string ( d_->getPort() ) << std::endl;

        d_->setOnPeerAddedHandler ( [] ( const Discovery::Peer& peer ) {
            std::cout << "Received new peer: ["
                << boost::lexical_cast<std::string> ( peer.id )
                << "] on port " << std::to_string ( peer.port )
                << " with addresses: ";

            for ( size_t i = 0; i < peer.addrs.size(); ++i )
            {
                if ( i > 0 )
                    std::cout << ", ";

                std::cout << peer.addrs.at ( i ).to_string();
            }

            std::cout << std::endl;
         } );

        d_->setOnPeerUpdatedHandler ( [] ( const Discovery::Peer& peer ) {
            std::cout << "Received update for peer: ["
                << boost::lexical_cast<std::string> ( peer.id ) << "]" << std::endl;
        } );

        d_->setOnPeerRemovedHandler ( [] ( const boost::uuids::uuid& id ) {
            std::cout << "Removed peer [" << boost::lexical_cast<std::string> ( id )
                << "]" << std::endl;
        } );

        d_->start();

        svc_.run();
    }

private:
    boost::asio::io_service svc_;

    DiscoveryPtr d_;
};

int main()
{
    srand ( time ( nullptr ) );

    TestDiscovery td;
    td.run();
}

