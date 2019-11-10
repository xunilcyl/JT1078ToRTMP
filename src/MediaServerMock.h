#pragma once
#include "IMediaServer.h"
#include <cstdio>

class MediaServerMock : public IMediaServer
{
public:
    int Start() override { return 0; }
    int Run() override { getchar(); return 0; }
    void Stop() override {}
    int GetPort() override { return 30000; }
};