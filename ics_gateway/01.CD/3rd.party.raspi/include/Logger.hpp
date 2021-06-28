#pragma once
#include<iostream>
#include<spdlog/spdlog.h>
#include"Utility/Singleton.hpp"

#define LOG_FILE_MODE_RELEASE "Release"


namespace Utility
{
	class Logger : public Singleton<Logger>
	{
	public:
		enum LogType
		{
			LOGTYPE_TRACE,
			LOGTYPE_DEBUG,
			LOGTYPE_INFO,
			LOGTYPE_WARNING,
			LOGTYPE_ERROR,
			LOGTYPE_CRITICAL,
			LOGTYPE_TRACE_RELEASE,
			LOGTYPE_DEBUG_RELEASE,
			LOGTYPE_INFO_RELEASE,
			LOGTYPE_WARNING_RELEASE,
			LOGTYPE_ERROR_RELEASE,
			LOGTYPE_CRITICAL_RELEASE,	
		};
	public:
		Logger()
		{			
		}
		~Logger()
		{

		}
		template <typename... Args>
		void Log(LogType type, Args&&... args)
		{
			if (!logger_) {
				//std::cerr << "logger not yet init" << std::endl;
				return;
			}

			try
			{
				switch (type)
				{
				case LOGTYPE_CRITICAL:
					logger_->critical(std::forward<Args>(args)...);
					break;
				case LOGTYPE_ERROR:
					logger_->error(std::forward<Args>(args)...);
					break;
				case LOGTYPE_WARNING:
					logger_->warn(std::forward<Args>(args)...);
					break;
				case LOGTYPE_INFO:
					logger_->info(std::forward<Args>(args)...);
					break;
				case LOGTYPE_DEBUG:
					logger_->debug(std::forward<Args>(args)...);
					break;
				case LOGTYPE_TRACE:
					logger_->trace(std::forward<Args>(args)...);
					break;
				default:
					std::cerr << "Wrong type" << std::endl;
					break;

				}
			}
			catch (const spdlog::spdlog_ex &e)
			{
				std::cerr << "spd exception " << e.what() << std::endl;				
			}
			catch (...)
			{
				std::cerr << "Unknow exception" << std::endl;				
			}
		}
		
		template <typename... Args>
		void LogRelease(LogType type, Args&&... args)
		{
			if (!logger_RELEASE)
			{
				std::cerr << "logger release not yet init" << std::endl;
				return;
			}
			try
			{
				switch (type)
				{
				case LOGTYPE_CRITICAL_RELEASE:
					logger_RELEASE->critical(std::forward<Args>(args)...);
					break;
				case LOGTYPE_ERROR_RELEASE:
					logger_RELEASE->error(std::forward<Args>(args)...);
					break;
				case LOGTYPE_WARNING_RELEASE:
					logger_RELEASE->warn(std::forward<Args>(args)...);
					break;
				case LOGTYPE_INFO_RELEASE:
					logger_RELEASE->info(std::forward<Args>(args)...);
					break;
				case LOGTYPE_DEBUG_RELEASE:
					logger_RELEASE->debug(std::forward<Args>(args)...);
					break;
				case LOGTYPE_TRACE_RELEASE:
					logger_RELEASE->trace(std::forward<Args>(args)...);
					std::cout << "fuck";
					break;
				default:
					std::cerr << "Wrong type" << std::endl;
					break;

				}
			}
			catch (const spdlog::spdlog_ex &e)
			{
				std::cerr << "spd exception " << e.what() << std::endl;				
			}
			catch (...)
			{
				std::cerr << "Unknow exception" << std::endl;				
			}
		}

		void Start(const std::string &name, const std::string &filename, int size, int level = 0, bool bconsole = false)
		{
			if (name == LOG_FILE_MODE_RELEASE)
			{
				try
				{
					if (bconsole)
					{
						logger_RELEASE = spdlog::stdout_logger_mt(name);
					}
					else
					{
						logger_RELEASE = spdlog::rotating_logger_mt(name, filename, size, 10, true);
					}
					logger_RELEASE->set_level((spdlog::level::level_enum)level);
				}
				catch (const spdlog::spdlog_ex &e)
				{
					std::cerr << "spd exception " << e.what() << std::endl;
					logger_RELEASE.reset();
				}
				catch(const std::exception &e)
				{
					std::cerr << "exception " << e.what() << std::endl;
					logger_RELEASE.reset();
				}
				catch (...)
				{
					std::cerr << "unknow error" << std::endl;
					logger_RELEASE.reset();
				}
			}
			else
			{
				try
				{
					if (bconsole)
					{
						logger_ = spdlog::stdout_logger_mt(name);
					}
					else
					{
						logger_ = spdlog::rotating_logger_mt(name, filename, size, 10, true);
					}
					logger_->set_level((spdlog::level::level_enum)level);
				}
				catch (const spdlog::spdlog_ex &e)
				{
					std::cerr << "spd exception " << e.what() << std::endl;
					logger_.reset();
				}
				catch(const std::exception &e)
				{
					std::cerr << "exception " << e.what() << std::endl;
					logger_.reset();
				}
				catch (...)
				{
					std::cerr << "unknow error" << std::endl;
					logger_.reset();
				}
			}
		}

	private:
		std::shared_ptr<spdlog::logger> logger_;
		std::shared_ptr<spdlog::logger> logger_RELEASE;
	};

}
#define LOGGER_TRACE(...) Utility::Logger::getInstance().Log(Utility::Logger::LOGTYPE_TRACE, __VA_ARGS__)
#define LOGGER_DEBUG(...) Utility::Logger::getInstance().Log(Utility::Logger::LOGTYPE_DEBUG, __VA_ARGS__)
#define LOGGER_ERROR(...) Utility::Logger::getInstance().Log(Utility::Logger::LOGTYPE_ERROR, __VA_ARGS__)
#define LOGGER_INFO(...) Utility::Logger::getInstance().Log(Utility::Logger::LOGTYPE_INFO, __VA_ARGS__)
#define LOGGER_WARN(...) Utility::Logger::getInstance().Log(Utility::Logger::LOGTYPE_WARNING, __VA_ARGS__)
#define LOGGER_CRITICAL(...) Utility::Logger::getInstance().Log(Utility::Logger::LOGTYPE_CRITICAL, __VA_ARGS__)
#define LOGGER_TRY_CATCH(name,x) try { x; } catch (const std::exception &e) {LOGGER_ERROR(name,e.what());} catch(...){LOGGER_ERROR(name,"Unknow exception");}
#define LOGGER_CATCH(name) catch (const std::exception &e) {LOGGER_ERROR(name,e.what());} catch(...){LOGGER_ERROR(name,"Unknow exception");}

#define LOGGER_TRACE_RELEASE(...) Utility::Logger::getInstance().LogRelease(Utility::Logger::LOGTYPE_TRACE_RELEASE, __VA_ARGS__)
#define LOGGER_DEBUG_RELEASE(...) Utility::Logger::getInstance().LogRelease(Utility::Logger::LOGTYPE_DEBUG_RELEASE, __VA_ARGS__)
#define LOGGER_ERROR_RELEASE(...) Utility::Logger::getInstance().LogRelease(Utility::Logger::LOGTYPE_ERROR_RELEASE, __VA_ARGS__)
#define LOGGER_INFO_RELEASE(...) Utility::Logger::getInstance().LogRelease(Utility::Logger::LOGTYPE_INFO_RELEASE, __VA_ARGS__)
#define LOGGER_WARN_RELEASE(...) Utility::Logger::getInstance().LogRelease(Utility::Logger::LOGTYPE_WARNING_RELEASE, __VA_ARGS__)
#define LOGGER_CRITICAL_RELEASE(...) Utility::Logger::getInstance().LogRelease(Utility::Logger::LOGTYPE_CRITICAL_RELEASE, __VA_ARGS__)
#define LOGGER_TRY_CATCH_RELEASE(name,x) try { x; } catch (const std::exception &e) {LOGGER_ERROR_RELEASE(name,e.what());} catch(...){LOGGER_ERROR_RELEASE(name,"Unknow exception");}
#define LOGGER_CATCH_RELEASE(name) catch (const std::exception &e) {LOGGER_ERROR_RELEASE(name,e.what());} catch(...){LOGGER_ERROR_RELEASE(name,"Unknow exception");}





