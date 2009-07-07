//
// Automatically generated by ipdlc.
// Edit at your own risk
//

#ifndef mozilla_plugins_NPAPIProtocol_h
#define mozilla_plugins_NPAPIProtocol_h

#include "base/basictypes.h"
#include "nscore.h"
#include "IPC/IPCMessageUtils.h"
#include "mozilla/ipc/MessageTypes.h"
#include "mozilla/ipc/ProtocolUtils.h"
#include "npapi.h"
#include "mozilla/plugins/NPPProtocol.h"

namespace mozilla {
namespace plugins {
namespace NPAPIProtocol {


enum State {
};

enum NPAPIProtocolMsgType {
    NPAPIProtocolStart = NPAPIProtocolMsgStart << 12,
    NPAPIProtocolPreStart = (NPAPIProtocolMsgStart << 12) - 1,
    Msg_NP_Initialize__ID,
    Reply_NP_Initialize__ID,
    Msg_NPPConstructor__ID,
    Reply_NPPConstructor__ID,
    Msg_NPPDestructor__ID,
    Reply_NPPDestructor__ID,
    NPAPIProtocolEnd
};
class Msg_NP_Initialize :
    public IPC::Message
{
private:
    typedef mozilla::ipc::String String;
    typedef mozilla::ipc::StringArray StringArray;

public:
    enum {
        ID = Msg_NP_Initialize__ID
    };
    Msg_NP_Initialize() :
        IPC::Message(MSG_ROUTING_NONE, ID, PRIORITY_NORMAL)
    {
    }

    static bool Read(const Message* msg)
    {
        return true;
    }
};
class Reply_NP_Initialize :
    public IPC::Message
{
private:
    typedef mozilla::ipc::String String;
    typedef mozilla::ipc::StringArray StringArray;

public:
    enum {
        ID = Reply_NP_Initialize__ID
    };
    Reply_NP_Initialize(const NPError& rv) :
        IPC::Message(MSG_ROUTING_NONE, ID, PRIORITY_NORMAL)
    {
        IPC::WriteParam(this, rv);
    }

    static bool Read(
                const Message* msg,
                NPError* rv)
    {
        void* iter = 0;

        if (!(IPC::ReadParam(msg, &(iter), rv))) {
            return false;
        }

        return true;
    }
};
class Msg_NPPConstructor :
    public IPC::Message
{
private:
    typedef mozilla::ipc::String String;
    typedef mozilla::ipc::StringArray StringArray;

public:
    enum {
        ID = Msg_NPPConstructor__ID
    };
    Msg_NPPConstructor(
                const String& aMimeType,
                const uint16_t& aMode,
                const StringArray& aNames,
                const StringArray& aValues,
                const mozilla::ipc::ActorHandle& __ah) :
        IPC::Message(MSG_ROUTING_NONE, ID, PRIORITY_NORMAL)
    {
        IPC::WriteParam(this, aMimeType);
        IPC::WriteParam(this, aMode);
        IPC::WriteParam(this, aNames);
        IPC::WriteParam(this, aValues);
        IPC::WriteParam(this, __ah);
    }

    static bool Read(
                const Message* msg,
                String* aMimeType,
                uint16_t* aMode,
                StringArray* aNames,
                StringArray* aValues,
                mozilla::ipc::ActorHandle* __ah)
    {
        void* iter = 0;

        if (!(IPC::ReadParam(msg, &(iter), aMimeType))) {
            return false;
        }

        if (!(IPC::ReadParam(msg, &(iter), aMode))) {
            return false;
        }

        if (!(IPC::ReadParam(msg, &(iter), aNames))) {
            return false;
        }

        if (!(IPC::ReadParam(msg, &(iter), aValues))) {
            return false;
        }

        if (!(IPC::ReadParam(msg, &(iter), __ah))) {
            return false;
        }

        return true;
    }
};
class Reply_NPPConstructor :
    public IPC::Message
{
private:
    typedef mozilla::ipc::String String;
    typedef mozilla::ipc::StringArray StringArray;

public:
    enum {
        ID = Reply_NPPConstructor__ID
    };
    Reply_NPPConstructor(
                const NPError& rv,
                const mozilla::ipc::ActorHandle& __ah) :
        IPC::Message(MSG_ROUTING_NONE, ID, PRIORITY_NORMAL)
    {
        IPC::WriteParam(this, rv);
        IPC::WriteParam(this, __ah);
    }

    static bool Read(
                const Message* msg,
                NPError* rv,
                mozilla::ipc::ActorHandle* __ah)
    {
        void* iter = 0;

        if (!(IPC::ReadParam(msg, &(iter), rv))) {
            return false;
        }

        if (!(IPC::ReadParam(msg, &(iter), __ah))) {
            return false;
        }

        return true;
    }
};
class Msg_NPPDestructor :
    public IPC::Message
{
private:
    typedef mozilla::ipc::String String;
    typedef mozilla::ipc::StringArray StringArray;

public:
    enum {
        ID = Msg_NPPDestructor__ID
    };
    Msg_NPPDestructor(const mozilla::ipc::ActorHandle& __ah) :
        IPC::Message(MSG_ROUTING_NONE, ID, PRIORITY_NORMAL)
    {
        IPC::WriteParam(this, __ah);
    }

    static bool Read(
                const Message* msg,
                mozilla::ipc::ActorHandle* __ah)
    {
        void* iter = 0;

        if (!(IPC::ReadParam(msg, &(iter), __ah))) {
            return false;
        }

        return true;
    }
};
class Reply_NPPDestructor :
    public IPC::Message
{
private:
    typedef mozilla::ipc::String String;
    typedef mozilla::ipc::StringArray StringArray;

public:
    enum {
        ID = Reply_NPPDestructor__ID
    };
    Reply_NPPDestructor(
                const NPError& rv,
                const mozilla::ipc::ActorHandle& __ah) :
        IPC::Message(MSG_ROUTING_NONE, ID, PRIORITY_NORMAL)
    {
        IPC::WriteParam(this, rv);
        IPC::WriteParam(this, __ah);
    }

    static bool Read(
                const Message* msg,
                NPError* rv,
                mozilla::ipc::ActorHandle* __ah)
    {
        void* iter = 0;

        if (!(IPC::ReadParam(msg, &(iter), rv))) {
            return false;
        }

        if (!(IPC::ReadParam(msg, &(iter), __ah))) {
            return false;
        }

        return true;
    }
};


} // namespace NPAPIProtocol
} // namespace plugins
} // namespace mozilla

#endif // ifndef mozilla_plugins_NPAPIProtocol_h
