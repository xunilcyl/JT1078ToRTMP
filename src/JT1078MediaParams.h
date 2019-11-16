#pragma once
#include "Common.h"
#include "Packet.h"

constexpr uint8 VIDOE_I_FRAME     = 0x0;
constexpr uint8 VIDOE_P_FRAME     = 0x1;
constexpr uint8 VIDOE_B_FRAME     = 0x2;
constexpr uint8 AUDIO_FRAME       = 0x3;
constexpr uint8 PASS_THROUGH_DATA = 0x4;

// sizeof PacketHeaderBase should 16 bytes
struct PacketHeaderBase
{
    uint8 frameHeaderIdentify[4];
    uint8 cc:4;
    uint8 x:1;
    uint8 p:1;
    uint8 version:2;
    uint8 pt:7;
    uint8 m:1;
    uint16 seq;
    uint8 sim[6];
    uint8 chnNo;
    uint8 slicePacket:4;
    uint8 dataType:4;
};

// sizeof VideoPacketHeader is 30
struct VideoPacketHeader : public PacketHeaderBase
{
    uint8 timeStamp[8];              // ms
    uint16 lastIFrameInterval;       // ms
    uint16 lastFrameInterval;        // ms
    uint16 bodyLength;
};

// sizeof AudioPacketHeader is 26
struct AudioPacketHeader : public PacketHeaderBase
{
    uint8 timeStamp[8];              // ms
    uint16 bodyLength;
};

// sizeof PassThroughPacketHeader is 18
struct PassThroughPacketHeader : public PacketHeaderBase
{
    uint16 bodyLength;
};

struct JTPacket : public Packet
{
    JTPacket() : m_packetHeader(nullptr) {}
    
    PacketHeaderBase* m_packetHeader;
};

constexpr int VIDEO_HEADER_SIZE = sizeof(VideoPacketHeader);
constexpr int AUDIO_HEADER_SIZE = sizeof(AudioPacketHeader);
constexpr int PASS_THROUGH_HEADER_SIZE = sizeof(PassThroughPacketHeader);
constexpr int MIN_HEADER_SIZE = PASS_THROUGH_HEADER_SIZE;


static bool IsVideoPacket(uint8 type)
{
    return type == VIDOE_I_FRAME
        || type == VIDOE_P_FRAME
        || type == VIDOE_B_FRAME;
}

static bool IsAudioPacket(uint8 type)
{
    return type == AUDIO_FRAME;
}