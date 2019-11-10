#pragma once

#include <string>
#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(severity_logger_mt_tag, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>);

#define LOG_SEVERITY(severity) \
	BOOST_LOG_SEV(severity_logger_mt_tag::get(), severity) \
		<< __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << "() "

// These macro family generate different severity log to console or/and file.
// Trace is the least vital level and FATAL is the most critical level.

#define LOG_TRACE \
	LOG_SEVERITY(boost::log::trivial::trace)

#define LOG_DEBUG \
	LOG_SEVERITY(boost::log::trivial::debug)

#define LOG_INFO  \
	LOG_SEVERITY(boost::log::trivial::info)

#define LOG_WARN \
	LOG_SEVERITY(boost::log::trivial::warning)

#define LOG_ERROR \
	LOG_SEVERITY(boost::log::trivial::error)

#define LOG_FATAL \
	LOG_SEVERITY(boost::log::trivial::fatal)

namespace common {

	constexpr int kInvalidId = -1;

	bool InitLogger(int id = kInvalidId, const std::string& path = "./log");
	void UnInitLogger();
	void SetLogLevel(boost::log::trivial::severity_level level);
}