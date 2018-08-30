//
// Created by ts on 18-2-7.
//


#include <cstdlib>
#include <ctime>
#include "minilogger.h"

std::ofstream Logger::m_error_log_file;
std::ofstream Logger::m_info_log_file;
std::ofstream Logger::m_warn_log_file;

void initLogger(const std::string&info_log_filename,
                const std::string&warn_log_filename,
                const std::string&error_log_filename){
    Logger::m_info_log_file.open(info_log_filename.c_str());
    Logger::m_warn_log_file.open(warn_log_filename.c_str());
    Logger::m_error_log_file.open(error_log_filename.c_str());
}

std::ostream& Logger::getStream(log_rank_t log_rank){
    return (LOG_INFO == log_rank) ?
           (m_info_log_file.is_open() ?m_info_log_file : std::cout) :
           (LOG_WARNING == log_rank ?
            (m_warn_log_file.is_open()? m_warn_log_file : std::cerr) :
            (m_error_log_file.is_open()? m_error_log_file : std::cerr));
}

std::ostream& Logger::start(log_rank_t log_rank,
                            const int line,
                            const std::string&function)
{
    time_t tm;
    time(&tm);
    char time_string[128];
#ifdef WIN32
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S",localtime(&tm) );
#else
    ctime_r(&tm, time_string);
#endif
    return getStream(log_rank) << time_string
                               << "function (" << function << ")"
                               << "line " << line<<" "
                               <<std::flush;
}

 Logger::Logger(log_rank_t log_rank):m_log_rank(log_rank)
 {

 }

Logger::~Logger(){
    getStream(m_log_rank) << std::endl << std::flush;

    if (LOG_FATAL == m_log_rank) {
        m_info_log_file.close();
        m_info_log_file.close();
        m_info_log_file.close();
        abort();
    }
}
