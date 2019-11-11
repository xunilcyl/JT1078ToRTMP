#include "HttpServer.h"
#include "IMediaServerManager.h"
#include "Logger.h"
#include <microhttpd.h>

constexpr char UNSUPPORT_METHOD[] = "Unsupport method";
constexpr char INVALID_PARAMS[] = "Invalid Params";
constexpr char ALLOC_PORT_EXIST[] = "Port Already Exist";
constexpr char DEALLOC_NO_PORT[] = "Can't find media server";

//static
int HttpServer::HandleRequestCallback(
    void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **ptr)
{
    assert(cls);
    HttpServer* pthis = static_cast<HttpServer*>(cls);
    return pthis->HandleRequest(connection, url, method, version, upload_data, upload_data_size, ptr);
}

static void request_completed_callback(
    void *cls,
    struct MHD_Connection *connection,
    void **con_cls,
    enum MHD_RequestTerminationCode toe)
{
    ConnectionInfoPtr connInfo = (ConnectionInfoPtr)*con_cls;
    if (connInfo) {
        LOG_INFO << "free connInfo " << connInfo;
        delete connInfo;
        *con_cls = NULL;
    }
    LOG_INFO << "Connection " << connection << " serve complete";
}

static int PrintRequestHeaderCallback(
    void *cls,
    enum MHD_ValueKind kind, 
    const char *key,
     const char *value)
{
  LOG_INFO << key << " " << value;
  return MHD_YES;
}

//-------------------------------------------------------------------------//

int HttpServer::Start()
{
    m_daemon = MHD_start_daemon(
        MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
        8888,
        NULL, NULL,
        &HandleRequestCallback, (void*)this,
        MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 15,
        MHD_OPTION_NOTIFY_COMPLETED,
        &request_completed_callback, NULL,
        MHD_OPTION_END);

    if (m_daemon == NULL) {
        LOG_ERROR << "Start HTTP manager server failed";
        return -1;
    }

    LOG_INFO << "Start HTTP manager server successfully";

    return 0;
}

int HttpServer::Stop()
{
    LOG_INFO << "Stop HTTP manager server";

    if (m_daemon) {
        MHD_stop_daemon(m_daemon);
    }

    return 0;
}

int HttpServer::HandleRequest(
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *upload_data,
    size_t *upload_data_size,
    void **ptr)
{
    LOG_DEBUG << method << " " << url << " " << version << " at connection " << connection;

    ConnectionInfoPtr connInfo = (ConnectionInfoPtr)*ptr;

    if (connInfo == NULL) {
        PrintRequestHeader(connection);
        *ptr = CreateConnectionInfo();
        LOG_INFO << "connInfo " << *ptr;

        return MHD_YES;
    }

    std::string strMethod = method;
    if (strMethod == MHD_HTTP_METHOD_POST) {
        return HandlePost(connection, url, upload_data, upload_data_size, connInfo);
    }
    else {
        return HandleUnsupportMethod(connection);
    }
}

int HttpServer::HandlePost(
    struct MHD_Connection *connection,
    const char *url,
    const char *upload_data,
    size_t *upload_data_size,
    ConnectionInfoPtr connInfo)
{
    auto& dataSize = *upload_data_size;

    if (dataSize > 0) {
        if (ConsumeData(connInfo, upload_data, upload_data_size) < 0) {
            return MHD_NO;
        }
    }
    else {
        return OnReceiveAllData(connection, connInfo);
    }

    return MHD_YES;
}

int HttpServer::HandleUnsupportMethod(struct MHD_Connection *connection)
{
    return ResponseWithContent(connection, UNSUPPORT_METHOD, strlen(UNSUPPORT_METHOD), MHD_HTTP_NOT_IMPLEMENTED);
}

int HttpServer::ResponseWithContent(struct MHD_Connection *connection, const char* content, int length, unsigned int statusCode/* = MHD_HTTP_OK*/)
{
    auto response = MHD_create_response_from_buffer(
        length,
        (void*)content,
        MHD_RESPMEM_PERSISTENT);
    
    if (response == NULL) {
        LOG_ERROR << "Create response failed for " << connection;
        return MHD_NO;
    }

    int ret = MHD_queue_response(connection, statusCode, response);
    MHD_destroy_response(response);

    return ret;
}

void HttpServer::PrintRequestHeader(struct MHD_Connection *connection)
{
    // if debug on
    MHD_get_connection_values(connection, MHD_HEADER_KIND, &PrintRequestHeaderCallback, NULL);
}

ConnectionInfoPtr HttpServer::CreateConnectionInfo()
{
    ConnectionInfoPtr connInfo = new ConnectionInfo;
    memset(connInfo->buffer, 0, sizeof(connInfo->buffer));
    connInfo->size = 0;
    return connInfo;
}

int HttpServer::ConsumeData(ConnectionInfoPtr connInfo, const char *upload_data, size_t *upload_data_size)
{
    auto& dataSize = *upload_data_size;
    if (connInfo->size + dataSize >= sizeof(connInfo->buffer)) {
        LOG_ERROR << "Receive too many data in one connection. Limit buffer size is " << sizeof(connInfo->buffer);
        return -1;
    }

    memcpy(connInfo->buffer + connInfo->size, upload_data, dataSize);
    connInfo->size += dataSize;
    dataSize = 0;

    return 0;
}

int HttpServer::OnReceiveAllData(struct MHD_Connection *connection, ConnectionInfoPtr connInfo)
{
    LOG_DEBUG << "connection " << connection << " receive data size: " << connInfo->size;

	return m_requestParser.Parse(connInfo->buffer, (void*)connection);
}

int HttpServer::requestAllocateMediaPort(const std::string& uniqueID, int seqID, void* userData)
{
	LOG_INFO << "uniqueID: " << uniqueID << ", seqID: " << seqID;

    auto iter = m_mediaPorts.find(uniqueID);
    if (iter != m_mediaPorts.end()) {
        return ResponseWithContent((struct MHD_Connection*)userData, ALLOC_PORT_EXIST, strlen(ALLOC_PORT_EXIST));
    }
    
    auto port = m_mediaServerManager.GetPort();
    if (port > 0) {
        m_mediaPorts.emplace(uniqueID, port);
    }
    std::string respMsg = m_requestParser.EncodeAllocMediaPortResp(m_localIP, port, 0, seqID);
    return ResponseWithContent((struct MHD_Connection*)userData, respMsg.c_str(), respMsg.length());
}

int HttpServer::requestDeallocateMediaPort(const std::string& uniqueID, int seqID, void* userData)
{
    LOG_INFO << "uniqueID: " << uniqueID << ", seqID: " << seqID;

    auto iter = m_mediaPorts.find(uniqueID);
    if (iter == m_mediaPorts.end()) {
        LOG_WARN << "No media server for uniqueID " << uniqueID;
        return ResponseWithContent((struct MHD_Connection*)userData, DEALLOC_NO_PORT, strlen(DEALLOC_NO_PORT));
    }

    auto retVal = m_mediaServerManager.FreePort(iter->second);
    m_mediaPorts.erase(iter);

    std::string respMsg = m_requestParser.EncodeDeallocMediaPortResp(retVal, seqID);
    return ResponseWithContent((struct MHD_Connection*)userData, respMsg.c_str(), respMsg.length());
}

int HttpServer::requestParseError(void* userData)
{
    return ResponseWithContent((struct MHD_Connection*)userData, INVALID_PARAMS, strlen(INVALID_PARAMS));
}
