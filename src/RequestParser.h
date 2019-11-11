#pragma once
#include <string>

class IRequestHandler
{
public:
    virtual ~IRequestHandler() = default;
	virtual int requestAllocateMediaPort(const std::string& uniqueID, int seqID, void* userData) = 0;
	virtual int requestDeallocateMediaPort(const std::string& uniqueID, int seqID, void* userData) = 0;
    virtual int requestParseError(void* userData) = 0;
};

class RequestParser
{
public:
    RequestParser(IRequestHandler& requestHandler)
        : m_requestHandler(requestHandler)
    {
    }
    ~RequestParser() = default;

    int Parse(const std::string& data, void* userData);

    std::string EncodeAllocMediaPortResp(const std::string& ip, int port, int result, int seqID);
    std::string EncodeDeallocMediaPortResp(int result, int seqID);

private:
    IRequestHandler& m_requestHandler;
};


