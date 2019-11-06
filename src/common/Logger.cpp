#include "logger.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <windows.h>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using namespace boost::log;

namespace common {

	// Max size of process name, too long log file name is not expected
	constexpr std::size_t kMaxProcessNameLength = 100;

	// File rotation size. When file size reach kRotationSize, a new log file will be created
	constexpr unsigned int kRotationSize = 100 * 1024 * 1024;

	// Max total file size which match the file pattern
	constexpr unsigned int kMaxTotalSize = 500 * 1024 * 1024;

	std::string GetProcessName()
	{
		char buffer[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, buffer, sizeof(buffer));

		std::string fileName;
		std::string filePath(buffer);
		auto pos = filePath.find_last_of('\\');
		if (pos != std::string::npos) {
			fileName = filePath.substr(pos + 1);
		}
		else {
			fileName = filePath;
		}

		if (fileName.length() > kMaxProcessNameLength) {
			fileName = fileName.substr(0, kMaxProcessNameLength);
		}
		return fileName;
	}

	// pattern: exeName_processId_YYYYMMdd_hhmmss.log
	std::string constructFileNamePattern(const std::string& path)
	{
		std::string processName = GetProcessName();
		auto processId = GetCurrentProcessId();
		std::stringstream ss;
		ss << path << '/' << processName << '_' << processId << "_%Y%m%d_%H%M%S.log";
		std::string pattern = ss.str();
		return pattern;
	}

	// pattern: exeName_id_processId_YYYYMMdd_hhmmss.log
	std::string constructFileNamePatternById(int id, const std::string& path)
	{
		std::string processName = GetProcessName();
		auto processId = GetCurrentProcessId();
		std::stringstream ss;
		ss << path << '/' << processName << '_' << id << '_' << processId << "_%Y%m%d_%H%M%S.log";
		std::string pattern = ss.str();
		return pattern;
	}

	logging::formatter GetFormatter()
	{
		return expr::stream
			<< expr::attr< unsigned int >("RecordID") // First an attribute "RecordID" is written to the log
			<< " [" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
			<< "] " << expr::attr< attrs::current_thread_id::value_type >("ThreadID")
			<< " [" << expr::attr< trivial::severity_level >("Severity")
			<< "] " << expr::smessage;
	}

	void AddAttributes()
	{
		logging::core::get()->add_global_attribute("RecordID", attrs::counter< unsigned int >());
		logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
		logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());
		logging::core::get()->add_thread_attribute("Scope", attrs::named_scope());
	}

	bool InitLoggerImp(const std::string& path, const std::string& name)
	{
		using file_sink = sinks::synchronous_sink< sinks::text_file_backend >;
		boost::shared_ptr<file_sink> fileSink(
			new file_sink(
				keywords::file_name = name,
				keywords::rotation_size = kRotationSize,
				keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0)
			));

		{
			auto backend = fileSink->locked_backend();
			backend->set_file_collector(
				sinks::file::make_collector(
					keywords::target = path,                          // where to store rotated files
					keywords::max_size = kMaxTotalSize,                // maximum total size of the stored files, in bytes
					keywords::min_free_space = 1024 * 1024 * 1024,      // minimum free space on the drive, in bytes
					keywords::max_files = 512                          // maximum number of stored files
				));
			backend->scan_for_files();
			backend->auto_flush(true);
		}
		auto formatter = GetFormatter();
		fileSink->set_formatter(formatter);

		logging::core::get()->add_sink(fileSink);

		auto consoleSink = logging::add_console_log(std::clog);
		consoleSink->set_formatter(formatter);

		AddAttributes();

		return true;
	}

	bool InitLogger(int id, const std::string& path)
	{
		std::string pattern = id == -1 ? constructFileNamePattern(path) : constructFileNamePatternById(id, path);

		return InitLoggerImp(path, pattern);
	}

	void UnInitLogger()
	{
		logging::core::get()->remove_all_sinks();
	}

	void SetLogLevel(boost::log::trivial::severity_level level)
	{
		logging::core::get()->set_filter(expr::attr< trivial::severity_level >("Severity") >= level);
	}
}