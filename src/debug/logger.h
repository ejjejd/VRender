#pragma once
#include <cstdlib>
#include <fstream>
#include <type_traits>
#include <cstdarg>
#include <unordered_map>

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
		virtual void OnReceive(const LogSeverity severity, const std::string& message) = 0;
		virtual std::string FormatMessage(const char* format, va_list args) = 0;
	};

	class Logger
	{
	private:
		std::unordered_map<uint32_t, uint32_t> MessagesLookup;

		uint32_t LimitOfIterations = 5;
		bool EnableSpamCheck = true;

		std::ofstream OutputFile;

		std::unique_ptr<BasePrinter> Printer;

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

		inline void Update()
		{
			if (!EnableSpamCheck)
				return;

			std::vector<uint32_t> eraseQueue;

			for (auto&[id, it] : MessagesLookup)
			{
				if (it > LimitOfIterations)
				{
					eraseQueue.push_back(id);
					continue;
				}

				++it;
			}

			for(auto& e : eraseQueue)
				MessagesLookup.erase(e);
		}

		void Send(const LogSeverity severity, const char* format, ...);

		//Set limit in the interval between identical message
		inline void SetSpamSettings(const uint32_t limitOfIterations)
		{
			EnableSpamCheck = limitOfIterations != 0;
			LimitOfIterations = limitOfIterations;
		}

		template<typename T, typename std::enable_if_t<std::is_base_of_v<BasePrinter, T>>* = nullptr>
		inline void SetPrinter()
		{
			Printer = std::make_unique<T>();
		}
	};

	inline Logger GlobalLoggger;
}