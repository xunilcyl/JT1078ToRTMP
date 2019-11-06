#pragma once

#include "IServer.h"

class Server : public IServer
{
public:
    Server();
    int Start() override;
    void Stop() override;
};