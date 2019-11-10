#pragma once
#include <string>

class IRequestHandler
{
public:
    virtual ~IRequestHandler() = default;
	virtual void requestAllocateMediaPort(const std::string& uniqueID, int seqID) = 0;
	virtual void requestDeallocateMediaPort(const std::string& uniqueID, int seqID) = 0;
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


