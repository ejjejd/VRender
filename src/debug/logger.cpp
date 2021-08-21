#include "logger.h"

#include <sstream>
#include <ctime>

namespace debug
{
	std::string Logger::GenerateInfoString(const LogSeverity severity)
	{
		std::string severityStr;
		switch (severity)
		{
		#define SEV(x) case LogSeverity::x: severityStr = #x; break;
					LOG_SEVERITY_TABLE
		#undef SEV
		}

		std::time_t t = std::time(0);
		std::tm* now = std::localtime(&t);

		std::stringstream ss;
		ss << "[" << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "]";
		ss << "[" << severityStr << "]" << " ";

		return ss.str();
	}

	void Logger::Send(const std::string& message, const LogSeverity severity)
	{
		auto infoString = GenerateInfoString(severity);

		if (Printer)
			Printer->OnReceive(infoString + message + std::string("\n"), severity);


		auto formatedMessage = Printer->FormatMessage(message);

		OutputFile << infoString + formatedMessage + std::string("\n");
	}
}