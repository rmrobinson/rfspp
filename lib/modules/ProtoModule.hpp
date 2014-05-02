#pragma once

#include <algorithm>
#include <cassert>

#include "fs/Module.hpp"
#include "Controller.hpp"

namespace rfs
{

template<typename T>
class ProtoModule : public Module
{
public:
    ProtoModule ( Controller<ProtoModule<T>, T>& controller, const std::string& name )
        : Module ( controller.getFS(), name, 0 ), controller_ ( controller )
    {}

    virtual ~ProtoModule() {}

    inline const T& getState() const
    {
        return state_;
    }

    proto::RetCode update ( const T& newState )
    {
        (void) newState;
        return proto::NotImplemented;
    }

protected:
    virtual proto::RetCode read ( const FileHandle&, std::vector<char>& data,
                                  size_t, size_t offset )
    {
        if ( offset != 0 )
        {
            return proto::NotSupported;
        }

        data.clear();

        std::string tmp;
        if ( ! state_.SerializeToString ( &tmp ) )
        {
            return proto::MalformedMessage;
        }

        std::copy ( tmp.begin(), tmp.end(), std::back_inserter ( data ) );
        assert ( tmp.size() == data.size() );

        return proto::Success;
    }

    virtual proto::RetCode write ( const FileHandle&, const std::vector<char>& data,
                                   size_t, size_t offset )
    {
        if ( offset != 0 )
        {
            return proto::NotSupported;
        }

        T state;
        if ( ! state.ParseFromArray ( &data[0], data.size() ) )
        {
            return proto::MalformedMessage;
        }

        proto::RetCode rc = controller_.set ( *this, state );

        if ( rc == proto::Success )
        {
            state_ = state;
        }

        return rc;
    }

    virtual size_t size() const
    {
        return state_.ByteSize();
    }

private:
    Controller<ProtoModule<T>, T>& controller_; ///< The controller responsible for this module instance.

    T state_; ///< The current state of this module instance.
};

}

