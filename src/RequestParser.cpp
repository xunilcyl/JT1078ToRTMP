#include "RequestParser.h"
#include "Logger.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

constexpr char ACTION_ON_PLAY[] = "on_play";
constexpr char ACTION_ON_STOP[] = "on_stop";

using namespace boost::property_tree;

int RequestParser::Parse(const std::string& data, void* userData)
{
	if (data.empty()) {
		LOG_WARN << "data is empty";
		return m_requestHandler.requestParseError(userData);
	}

	ptree pt;
	std::stringstream ss(data);

	try {
		json_parser::read_json(ss, pt);
		std::string method = pt.get<std::string>("method");
		std::string uniqueID = pt.get<std::string>("params.uniqueID");
		int seqID = pt.get<int>("id");

		if (method == "allocMediaPortReq") {
			return m_requestHandler.requestAllocateMediaPort(uniqueID, seqID, userData);
		}
		else if (method == "deallocMediaPortReq") {
			return m_requestHandler.requestDeallocateMediaPort(uniqueID, seqID, userData);
		}
		else {
			m_requestHandler.requestParseError(userData);
		}
	}
	catch (ptree_error& e) {
		LOG_ERROR << "Parse request failed. " << e.what();
		return m_requestHandler.requestParseError(userData);
	}
}

int RequestParser::ParseRtmpNotify(const std::string& data, void* userData)
{
	if (data.empty()) {
		LOG_WARN << "data is empty";
		return m_requestHandler.requestParseError(userData);
	}

	LOG_DEBUG << data;

	ptree pt;
	std::stringstream ss(data);

	try {
		json_parser::read_json(ss, pt);
		std::string action = pt.get<std::string>("action");
		std::string stream = pt.get<std::string>("stream");

		if (action == ACTION_ON_PLAY) {
			return m_requestHandler.notifyRtmpPlay(stream, userData);
		}
		else if (action == ACTION_ON_STOP) {
			return m_requestHandler.notifyRtmpStop(stream, userData);
		}
		else {
			return m_requestHandler.requestParseError(userData);
		}
	}
	catch (ptree_error& e) {
		LOG_ERROR << "Parse request failed. " << e.what();
		return m_requestHandler.requestParseError(userData);
	}
}

std::string RequestParser::EncodeAllocMediaPortResp(const std::string& ip, int port, int result, int seqID)
{
	std::string strRes((result == 0) ? "ok" : "failed");
	ptree pt;
	pt.put("method", "allocMediaPortResp");
	pt.put("params.ip", ip.c_str());
	pt.put<int>("params.port", port);
	pt.put("result", strRes.c_str());
	pt.put<int>("id", seqID);

	std::stringstream ss;
	json_parser::write_json(ss, pt);

	LOG_INFO << ss.str();

	return ss.str();
}

std::string RequestParser::EncodeDeallocMediaPortResp(int result, int seqID)
{
	std::string strRes((result == 0) ? "ok" : "failed");
	ptree pt;
	pt.put("method", "deallocMediaPortResp");
	pt.put("result", strRes.c_str());
	pt.put<int>("id", seqID);

	std::stringstream ss;
	json_parser::write_json(ss, pt);

	LOG_INFO << ss.str();

	return ss.str();
}
