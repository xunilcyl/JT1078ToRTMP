#include "MediaParser.h"
#include "Logger.h"
#include <cstring>

int MediaParser::Parse(const char* data, int size)
{
    if (size <= 0) {
        return -1;
    }

    if (m_size + size > sizeof(m_buffer)) {
        LOG_ERROR << "Buffer is full";
        return -1;
    }

    memcpy(m_buffer + m_size, data, size);
    m_size += size;

    return 0;
}

int MediaParser::GetPacket(Packet* packet)
{
    if (!packet) {
        LOG_ERROR << "Invalid parameter";
        return -1;
    }
    packet->m_data = m_buffer;
    packet->m_size = m_size;
    m_size = 0;
    return 0;
}