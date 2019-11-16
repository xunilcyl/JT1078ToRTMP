#pragma once
#include "IMediaParser.h"

class MediaParser : public IMediaParser
{
public:
    MediaParser()
        : m_size(0)
    {}

    Packet* AllocatePacket() override { return new Packet; }
    void FreePacket(Packet* packet) override { if (packet) delete packet; }
    int Parse(const char* data, int size) override;
    int GetPacket(Packet* packet) override;

private:
    enum { MAX_BUFFER_SIZE = 10 * 1024 };
    char m_buffer[MAX_BUFFER_SIZE];
    int m_size;
};
 