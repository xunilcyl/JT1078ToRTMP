#pragma once
#include "Common.h"
#include "IMediaParser.h"
#include "JT1078MediaParams.h"

class JT1078MediaParser : public IMediaParser
{
public:
    JT1078MediaParser()
        : m_size(0), m_pos(0)
    {}

    Packet* AllocatePacket() override;
    void FreePacket(Packet* packet) override;
    int Parse(const char* data, int size) override;
    int GetPacket(Packet* packet) override;

    bool HasH264Payload(Packet* packet) override;
    bool HasAacPayload(Packet* packet) override;
    bool IsEndOfFrame(Packet* packet) override;

private:
    int GetPacketImpl(JTPacket* packet, uint16 bodyLen, uint16 headerSize);
    bool IsCompletePacket(uint16 hearderSize);

private:
    enum { MAX_BUFFER_SIZE = 65 * 1024 };
    char m_buffer[MAX_BUFFER_SIZE];
    uint32 m_size;
    uint32 m_pos;
    JTPacket m_packet;
};