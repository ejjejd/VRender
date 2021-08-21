#pragma once
#include <cstdlib>
#include <fstream>
#include <type_traits>

#define LOG(...) { printf(__VA_ARGS__); printf("\n"); }
#define TERMINATE_LOG(...) { printf(__VA_ARGS__); std::abort(); }

namespace debug
{
#define LOG_SEVERITY_TABLE\
	SEV(Common)\
	SEV(Warning)\
	SEV(Error)

	enum class LogSeverity
	{
#define SEV(x) x,
		LOG_SEVERITY_TABLE
#undef SEV
	};

	class BasePrinter
	{
	public:
		virtual void OnReceive(const std::string& message, const LogSeverity severity) = 0;
		virtual std::string FormatMessage(const std::string& message) = 0;
	};

	class Logger
	{
	private:
		std::unique_ptr<BasePrinter> Printer;

		std::ofstream OutputFile;

		std::string GenerateInfoString(const LogSeverity severity);
	public:
		inline bool Setup(const char* filepath)
		{
			OutputFile.open(filepath, std::ios::trunc);
			return OutputFile.is_open();
		}

		inline void Cleanup()
		{
			OutputFile.close();
		}

		void Send(const std::string& message, const LogSeverity severity);

		template<typename T, typename std::enable_if_t<std::is_base_of_v<BasePrinter, T>>* = nullptr>
		inline void SetPrinter()
		{
			Printer = std::make_unique<T>();
		}
	};

	inline Logger GlobalLoggger;
}