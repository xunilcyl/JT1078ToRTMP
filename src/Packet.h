#pragma once

enum PacketType
{
    PACKET_VIDEO,
    PACKET_AUDIO,
    PACKET_OTHER,
};

struct Packet
{
    Packet() : m_data(nullptr), m_size(0), m_type(PACKET_OTHER) {}
    
    const char* m_data;
    int m_size;
    PacketType m_type;
};