#include "RequestParser.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>

using namespace boost::property_tree;

int RequestParser::Parse(const std::string& data)
{
	if (data.empty()) {
		LOG_WARN << "data is empty";
		return -1;
	}

	ptree pt;
	std::stringstream ss(data);

	try {
		json_parser::read_json(ss, pt);
		std::string method = pt.get<std::string>("method");
		std::string uniqueID = pt.get<std::string>("params.uniqueID");
		int seqID = pt.get<int>("id");

		if (method == "allocMediaPortReq") {
			m_requestHandler.requestAllocateMediaPort(uniqueID, seqID);
		}
		else if (method == "deallocMediaPortReq") {
			m_requestHandler.requestDeallocateMediaPort(uniqueID, seqID);
		}
	}
	catch (ptree_error& e) {
		LOG_ERROr << "Parse request failed. " << e.what();
	}

    return 0;
}
