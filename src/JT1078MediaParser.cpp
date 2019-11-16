#include "JT1078MediaParser.h"
#include "Logger.h"
#include <arpa/inet.h>

Packet* JT1078MediaParser::AllocatePacket()
{
    return new JTPacket{};
}

void JT1078MediaParser::FreePacket(Packet* packet)
{
    if (packet) {
        JTPacket* jtpkt = static_cast<JTPacket*>(packet);
        delete jtpkt;
    }
}

int JT1078MediaParser::Parse(const char* data, int size)
{
    if (m_size + size > sizeof(m_buffer)) {
        LOG_ERROR << "Buffer overflow";
        return -1;
    }

    if (m_pos != 0) { // move the data to begining of buffer
        for (int i = 0; i < m_size; ++i) {
            m_buffer[i] = m_buffer[m_pos + i];
        }
        m_pos = 0;
    }

    memcpy(m_buffer+m_size, data, size);
    m_size += size;

    return 0;
}

int JT1078MediaParser::GetPacket(Packet* pkt)
{
    if (m_size < MIN_HEADER_SIZE) {
        return -1;
    }

    JTPacket* packet = static_cast<JTPacket*>(pkt);
    PacketHeaderBase* base = reinterpret_cast<PacketHeaderBase*>(m_buffer+m_pos);

    if (IsVideoPacket(base->dataType)) {
        if (!IsCompletePacket(VIDEO_HEADER_SIZE)) {
            return -1;
        }
        VideoPacketHeader* vpHeader = (VideoPacketHeader*)base;
        packet->m_packetHeader = vpHeader;
        packet->m_type = PACKET_VIDEO;
        return GetPacketImpl(packet, vpHeader->bodyLength, VIDEO_HEADER_SIZE);
    }
    else if (IsAudioPacket(base->dataType) && m_size >= AUDIO_HEADER_SIZE) {
        if (!IsCompletePacket(AUDIO_HEADER_SIZE)) {
            return -1;
        } 
        AudioPacketHeader* apHeader = (AudioPacketHeader*)base;
        packet->m_packetHeader = apHeader;
        packet->m_type = PACKET_AUDIO;
        return GetPacketImpl(packet, apHeader->bodyLength, AUDIO_HEADER_SIZE);
    }
    else {
        if (!IsCompletePacket(PASS_THROUGH_HEADER_SIZE)) {
            return -1;
        } 
        PassThroughPacketHeader* ptpHeader = (PassThroughPacketHeader*)base;
        packet->m_packetHeader = ptpHeader;
        packet->m_type = PACKET_OTHER;
        return GetPacketImpl(packet, ptpHeader->bodyLength, PASS_THROUGH_HEADER_SIZE);
    }
}

int JT1078MediaParser::GetPacketImpl(JTPacket* packet, uint16 bodyLen, uint16 headerSize)
{
    uint16 datalen = ntohs(bodyLen);
    uint32 remainDataSize = m_size - headerSize;
    if (remainDataSize < datalen) {
        return -1;
    }
    packet->m_data = m_buffer + m_pos + headerSize;
    packet->m_size = datalen;

    auto packetSize = headerSize + datalen;
    m_size -= packetSize;
    m_pos += packetSize;

    return 0;
}

bool JT1078MediaParser::IsCompletePacket(uint16 hearderSize)
{
    return m_size >= hearderSize;
}