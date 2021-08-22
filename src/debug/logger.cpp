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
		ss << "-[" << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "]";
		ss << "[" << severityStr << "]" << " ";

		return ss.str();
	}

	void Logger::Send(const LogSeverity severity, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		auto formatedMessage = Printer->FormatMessage(format, args);
		va_end(args);

		if (EnableSpamCheck)
		{
			uint32_t hash = std::hash<std::string>{}(formatedMessage);

			auto& findRes = MessagesLookup.find(hash);
			if (findRes == MessagesLookup.end())
				MessagesLookup[hash] = 0;
			else
				return;
		}


		auto infoString = GenerateInfoString(severity);

		auto writeString = infoString + formatedMessage + std::string("\n");

		if (Printer)
			Printer->OnReceive(severity, writeString);
		
		if (OutputFile.is_open())
			OutputFile << writeString;
	}
}