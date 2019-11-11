#pragma once
#include "IMediaServerManager.h"
#include <cstdio>

class MediaServerManagerMock : public IMediaServerManager
{
public:
    int Start() override { return 0; }
    void Stop() override {}
    int GetPort() override { return 30000; }
    int FreePort(int port) { return 0; }
};