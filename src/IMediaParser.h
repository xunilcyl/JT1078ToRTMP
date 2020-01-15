#pragma once
#include "Packet.h"
#include <memory>

class IMediaParser
{
public:
    virtual ~IMediaParser() = default;
    virtual Packet* AllocatePacket() = 0;
    virtual void FreePacket(Packet* packet) = 0;
    virtual int Parse(const char* data, int size) = 0;
    virtual int GetPacket(Packet* packet) = 0;

    virtual bool HasH264Payload(Packet* packet) = 0;
    virtual bool HasAacPayload(Packet* packet) = 0;
    virtual bool IsEndOfFrame(Packet* packet) = 0;
};
