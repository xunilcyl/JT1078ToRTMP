#include "HttpServer.h"
#include "IConfiguration.h"
#include "IMediaServerManager.h"
#include "INotifier.h"
#include "Logger.h"
#include <microhttpd.h>

constexpr char RESPONSE_UNSUPPORT_METHOD[] = "Unsupport method";
constexpr char RESPONSE_RTMP_NOTIFY_OK[] = "0";

constexpr char URL_MEDIA[] = "/media";
constexpr char URL_RTMP_NOTIFY[] = "/rtmpNotify";

enum ErrorCode
{
    ERR_OK                              = 0,
    ERR_FAILED                          = 100,
    ERR_INVLIAD_PARAM                   = 101,
    ERR_PORT_EXIST_FOR_THIS_UNIQUE_ID   = 102,
    ERR_FIND_NO_PORT_OF_THIS_UNIQUE_ID  = 103,
};

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
        LOG_ERROR << "Start HTTP server failed";
        return -1;
    }

    LOG_INFO << "Start HTTP server successfully";

    return 0;
}

int HttpServer::Stop()
{
    if (m_daemon) {
        MHD_stop_daemon(m_daemon);
    }

    LOG_INFO << "HTTP server stopped";

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
    ConnectionInfoPtr connInfo = (ConnectionInfoPtr)*ptr;

    if (connInfo == NULL) {
        PrintRequestHeader(connection);

        std::string strUrl = url;
        if (strUrl == URL_MEDIA || strUrl == URL_RTMP_NOTIFY) {
            *ptr = CreateConnectionInfo();
            LOG_INFO << "New connection: " << connection << ", create connInfo : " << *ptr;
        }
        else {
            LOG_ERROR << "Unknow url:" << strUrl;
            return MHD_NO;
        }

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
        return OnReceiveAllData(connection, connInfo, url);
    }

    return MHD_YES;
}

int HttpServer::HandleUnsupportMethod(struct MHD_Connection *connection)
{
    return ResponseWithContent(connection, RESPONSE_UNSUPPORT_METHOD, strlen(RESPONSE_UNSUPPORT_METHOD), MHD_HTTP_NOT_IMPLEMENTED);
}

int HttpServer::ResponseWithContent(struct MHD_Connection *connection, const char* content, int length, unsigned int statusCode/* = MHD_HTTP_OK*/)
{
    auto response = MHD_create_response_from_buffer(
        length,
        (void*)content,
        MHD_RESPMEM_MUST_COPY);
    
    if (response == NULL) {
        LOG_ERROR << "Create response failed for " << connection;
        return MHD_NO;
    }

    int ret = MHD_queue_response(connection, statusCode, response);
    MHD_destroy_response(response);

    LOG_DEBUG << "queue response result: " << ret;

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

int HttpServer::OnReceiveAllData(struct MHD_Connection *connection, ConnectionInfoPtr connInfo, const char* url)
{
    LOG_DEBUG << "connection " << connection << " receive data size: " << connInfo->size;

    if (strcmp(url, URL_MEDIA) == 0) {
	    return m_requestParser.Parse(connInfo->buffer, (void*)connection);
    }
    else {
        return m_requestParser.ParseRtmpNotify(connInfo->buffer, (void*)connection);
    }
}

int HttpServer::requestAllocateMediaPort(const std::string& uniqueID, int seqID, void* userData)
{
    int port = -1;
    ErrorCode err = ERR_FAILED;

    if (uniqueID.empty()) {
        LOG_ERROR << "uniqueID is empty.";
        err = ERR_INVLIAD_PARAM;
    }
    else {
        auto iter = m_mediaPorts.find(uniqueID);
        if (iter != m_mediaPorts.end()) {
            LOG_WARN << "Port " << iter->second << " is already allocated for " << uniqueID;
            port = iter->second;
            err = ERR_PORT_EXIST_FOR_THIS_UNIQUE_ID;
        }
        else {
            port = m_mediaServerManager.GetPort(uniqueID);
            if (port > 0) {
                m_mediaPorts.emplace(uniqueID, port);
                err = ERR_OK;
            }
        }
    }
    
    LOG_INFO << "uniqueID: " << uniqueID << ", seqID: " << seqID << ", port: " << port;
    std::string respMsg = m_requestParser.EncodeAllocMediaPortResp(IConfiguration::Get().GetPublicIP(), port, err, seqID);
    return ResponseWithContent((struct MHD_Connection*)userData, respMsg.c_str(), respMsg.length());
}

int HttpServer::requestDeallocateMediaPort(const std::string& uniqueID, int seqID, void* userData)
{
    ErrorCode err = ERR_OK;

    if (uniqueID.empty()) {
        LOG_ERROR << "uniqueID is empty.";
        err = ERR_INVLIAD_PARAM;
    }
    else {
        auto iter = m_mediaPorts.find(uniqueID);

        if (iter == m_mediaPorts.end()) {
            LOG_WARN << "No media server for uniqueID " << uniqueID;
            err = ERR_FIND_NO_PORT_OF_THIS_UNIQUE_ID;
        }
        else {
            LOG_INFO << "uniqueID: " << uniqueID << ", seqID: " << seqID << ", port: " << iter->second;

            auto retVal = m_mediaServerManager.FreePort(iter->second);
            m_mediaPorts.erase(iter);
        }
    }
    
    std::string respMsg = m_requestParser.EncodeDeallocMediaPortResp(err, seqID);
    return ResponseWithContent((struct MHD_Connection*)userData, respMsg.c_str(), respMsg.length());
}

int HttpServer::requestParseError(void* userData)
{
    std::string respMsg = m_requestParser.EncodeGeneralResponse(ERR_INVLIAD_PARAM);
    return ResponseWithContent((struct MHD_Connection*)userData, respMsg.c_str(), respMsg.length());
}

int HttpServer::notifyRtmpPlay(const std::string& uniqueID, void* userData)
{
    LOG_INFO << "on_play " << uniqueID;
    m_notifier.NotifyRtmpPlay(uniqueID);
    return ResponseWithContent((struct MHD_Connection*)userData, RESPONSE_RTMP_NOTIFY_OK, strlen(RESPONSE_RTMP_NOTIFY_OK));
}

int HttpServer::notifyRtmpStop(const std::string& uniqueID, void* userData)
{
    LOG_INFO << "on_stop " << uniqueID;
    m_notifier.NotifyRtmpStop(uniqueID);
    return ResponseWithContent((struct MHD_Connection*)userData, RESPONSE_RTMP_NOTIFY_OK, strlen(RESPONSE_RTMP_NOTIFY_OK));
}
