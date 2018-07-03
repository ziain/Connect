#ifndef LwLogger_H
#define LwLogger_H
#include <map>
//#include "lwbase/LwErrorCode.h"
//#include "lwbase/LwDefine.h"

#ifdef _WIN32
#include <direct.h>
#define __LWCREATE_DIR _mkdir
#else
#include <sys/stat.h>
#define __LWCREATE_DIR mkdir
#endif

#if 0
#include "spdlog/spdlog.h"

///
/// \brief The LwLogger class
/// 日志类
class LwLogger{
public:
    ///
    /// \brief operator ()
    /// \param moduleName 模块名称。 同一个进程内不能有重复的模块名称！
    /// \param loggerName 日志类型
    /// \return 日记对象
    /// 日志记录函数。 使用方法如 LwLogger::inst()(LW_LOGGER_MODULE_PROTOCOL,LW_LOGGER_TAG_MAIN)->info("LwUnInit-Stop ret:{0}",ret);
    std::shared_ptr<spdlog::logger> operator ()(const char* moduleName=nullptr,const char* loggerName=nullptr){
        if(nullptr == moduleName || nullptr == loggerName){
            return mapLogger[LW_LOGGER_TAG_CONSOLE];
        }
        std::string logKey = std::string(moduleName)+loggerName;
        auto it = mapLogger.find(logKey);
        if(it != mapLogger.end()){
            return it->second;
        }else{
#if 0
            return mapLogger[LW_LOGGER_TAG_CONSOLE];
#else
            std::string dir = "logs/"+std::string(moduleName);
            __LWCREATE_DIR(dir.c_str());
            mapLogger[logKey] = spdlog::rotating_logger_mt(loggerName, dir+"/"+std::string(loggerName)+".txt", 1048576 * 5, 3);
            return  mapLogger[logKey];
#endif
        }
    }
    static LwLogger& inst(){
        static LwLogger ins;
        return ins;
    }
private:
    LwLogger(size_t qSize = 4096){

        auto console = spdlog::stdout_color_mt(LW_LOGGER_TAG_CONSOLE);
        console->info("Welcome to spdlog!");
        console->error("Some error message with arg{}..", 1);
        mapLogger[LW_LOGGER_TAG_CONSOLE] = console;
#if 0
        //        size_t q_size = 4096; // queue size must be power of 2
        spdlog::set_async_mode(qSize);
        spdlog::set_level(spdlog::level::trace);
        //        mapLogger[LW_LOGGER_TAG_MAIN] =  spdlog::rotating_logger_mt(LW_LOGGER_TAG_MAIN, "logs/lwvrprotocol.txt", 1048576 * 5, 3);
        //        mapLogger[LW_LOGGER_TAG_CLICK] = spdlog::rotating_logger_mt(LW_LOGGER_TAG_CLICK, "logs/click.txt", 1048576 * 5, 3);
#endif
    }
    std::map<std::string,std::shared_ptr<spdlog::logger>> mapLogger;
};
#else
#include "log4z.h"

#include <sstream>
#include <iostream>

using namespace zsummer::log4z;


class LwLogger{
public:
    LwLogger(){

//        logid_mysql = ILog4zManager::getRef().createLogger("mysql" );
//        logid_network = ILog4zManager::getRef().createLogger("network" );
//        logid_moniter = ILog4zManager::getRef().createLogger("moniter" );
        ILog4zManager::getRef().config("config.cfg");

//        ILog4zManager::getRef().setLoggerDisplay(logid_mysql, false);
//        ILog4zManager::getRef().setLoggerDisplay(logid_network, false);
//        ILog4zManager::getRef().setLoggerDisplay(logid_moniter, false);
        //ILog4zManager::getRef().setLoggerOutFile(logid_mysql, false);
        //ILog4zManager::getRef().setLoggerOutFile(logid_network, false);
        //ILog4zManager::getRef().setLoggerOutFile(logid_moniter, false);
//        ILog4zManager::getRef().setLoggerReserveTime(logid_mysql, 3*20);
//        ILog4zManager::getRef().setLoggerReserveTime(logid_network, 2*20);
//        ILog4zManager::getRef().setLoggerReserveTime(logid_moniter, 20);

        //        ILog4zManager::getRef().setLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_DEBUG);

        ILog4zManager::getRef().start();


    }

    static void LwPrintBin(char* buf, int size){
        printf("\nstart\n");
        char*p =buf;
        for(int i =0;i<size;i++){
           printf("%02x",*p++);
        }
        printf("\nend\n");
    }

    static LwLogger& inst(){
        static LwLogger ins;
        return ins;
    }
    LoggerId GetLogger(const char* key){
        LoggerId lid = ILog4zManager::getRef().findLogger(key);
        if(lid == LOG4Z_INVALID_LOGGER_ID){
           lid = ILog4zManager::getRef().createLogger(key);
        }
        return lid;
    }
    void setLoggerLevel(const char* key, int nLevel){
        ILog4zManager::getRef().setLoggerLevel(GetLogger(key), nLevel);
    }

private:
    ~LwLogger(){
        ILog4zManager::getRef().stop();
    }
};

#define LWLOGLEVEL(module) LwLogger::inst().setLoggerLevel(module);
#define LWLOGPRINT(module,log) std::cout<<module<<" "<<log<<std::endl

#define LWLOGT(module,log) LOG_TRACE(LwLogger::inst().GetLogger(module), "["<<module<<"] "<<log )
#define LWLOGD(module,log) LOG_DEBUG(LwLogger::inst().GetLogger(module), "["<<module<<"] "<<log )
#define LWLOGI(module,log) LOG_INFO(LwLogger::inst().GetLogger(module), "["<<module<<"] "<<log )
#define LWLOGW(module,log) LOG_WARN(LwLogger::inst().GetLogger(module), "["<<module<<"] "<<log )
#define LWLOGE(module,log) LOG_ERROR(LwLogger::inst().GetLogger(module), "["<<module<<"] "<<log )
#define LWLOGA(module,log) LOG_ALARM(LwLogger::inst().GetLogger(module), "["<<module<<"] "<<log )
#define LWLOGF(module,log) LOG_FATAL(LwLogger::inst().GetLogger(module), "["<<module<<"] "<<log )

#endif




#endif
