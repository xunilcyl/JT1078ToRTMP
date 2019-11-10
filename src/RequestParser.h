#pragma once
#include <string>

class IRequestHandler
{
public:
    virtual ~IRequestHandler() = default;
};

class RequestParser
{
public:
    RequestParser(IRequestHandler& requestHandler)
        : m_requestHandler(requestHandler)
    {
    }
    ~RequestParser() = default;

    int Parse(const std::string& data);

private:
    IRequestHandler& m_requestHandler;
};


