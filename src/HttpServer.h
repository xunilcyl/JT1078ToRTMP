#pragma once
#include "IHttpServer.h"
#include "RequestParser.h"
#include <map>
#include <memory>

struct MHD_Daemon;
class IMediaServerManager;

struct ConnectionInfo
{
    static constexpr int MAX_BUFFER_SIZE = 1024;

    char buffer[MAX_BUFFER_SIZE] = {0};
    int size;
};

typedef ConnectionInfo* ConnectionInfoPtr;
//typedef std::map<struct MHD_Connection*, ConnectionInfoPtr> ConnectionMap;

class HttpServer : public IHttpServer, public IRequestHandler
{
public:
    HttpServer(IMediaServerManager& mediaServer)
        : m_daemon(NULL)
        , m_mediaServerManager(mediaServer)
        , m_requestParser(*this)
        , m_localIP("127.0.0.1")
    {
    }

    int Start() override;
    int Stop() override;

private:
    static int HandleRequestCallback(
        void *cls,
        struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *version,
        const char *upload_data,
        size_t *upload_data_size,
        void **ptr);

    int HandleRequest(
        struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *version,
        const char *upload_data,
        size_t *upload_data_size,
        void **ptr);

    int HandlePost(
        struct MHD_Connection *connection,
        const char *url,
        const char *upload_data,
        size_t *upload_data_size,
        ConnectionInfoPtr connInfo);

    int HandleUnsupportMethod(struct MHD_Connection *connection);

    int ResponseWithContent(struct MHD_Connection *connection, const char* content, int length, unsigned int statusCode = 200);

    void PrintRequestHeader(struct MHD_Connection *connection);


    ConnectionInfoPtr CreateConnectionInfo();
    int ConsumeData(ConnectionInfoPtr connInfo, const char *upload_data, size_t *upload_data_size);
    int OnReceiveAllData(struct MHD_Connection *connection, ConnectionInfoPtr connInfo);

	// inherit from base classes
	int requestAllocateMediaPort(const std::string& uniqueID, int seqID, void* userData) override;
	int requestDeallocateMediaPort(const std::string& uniqueID, int seqID, void* userData) override;
    int requestParseError(void* userData) override;

private:
    struct MHD_Daemon* m_daemon;
    IMediaServerManager& m_mediaServerManager;
	RequestParser m_requestParser;
    std::string m_localIP;
    std::map<std::string, int> m_mediaPorts;
};
