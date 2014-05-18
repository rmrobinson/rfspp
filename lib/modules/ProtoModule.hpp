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

    RetCode update ( const T& newState )
    {
        (void) newState;
        return NotImplemented;
    }

protected:
    virtual RetCode read ( const FileHandle&, std::vector<char>& data,
                                  size_t, size_t offset )
    {
        if ( offset != 0 )
        {
            return NotSupported;
        }

        data.clear();

        std::string tmp;
        if ( ! state_.SerializeToString ( &tmp ) )
        {
            return MalformedMessage;
        }

        std::copy ( tmp.begin(), tmp.end(), std::back_inserter ( data ) );
        assert ( tmp.size() == data.size() );

        return Success;
    }

    virtual RetCode write ( const FileHandle&, const std::vector<char>& data,
                                   size_t, size_t offset )
    {
        if ( offset != 0 )
        {
            return NotSupported;
        }

        T state;
        if ( ! state.ParseFromArray ( &data[0], data.size() ) )
        {
            return MalformedMessage;
        }

        RetCode rc = controller_.set ( *this, state );

        if ( rc == Success )
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

