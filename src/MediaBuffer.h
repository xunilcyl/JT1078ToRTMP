#pragma once
#include <string.h>

class MediaBuffer
{
public:
    MediaBuffer(const char* data, int size) : m_data(NULL) , m_size(0) {
        if (size > 0) {
            m_size = size;
            m_data = new char[m_size];
            memcpy(m_data, data, m_size);
        }
    }
    ~MediaBuffer() {
        if (m_data) {
            delete[] m_data;
            m_data = NULL;
            m_size = 0;
        }
    }

public:
    char* m_data;
    int m_size;
};