/**
 *
 * Copyright (c) 2005-2012 POP-C++ project - GRID & Cloud Computing group, University of Applied Sciences of western Switzerland.
 * http://gridgroup.hefr.ch/popc
 *
 * @author Valenti Clement
 * @date 2012/07/05
 * @brief Define a proper function and interface to produce log from the POP-C++ runtime.
 *
 *
 * Modifications
 * - Refactor the popc_logger, added a static array for error messages (LW Jan 2015)
 */

#ifndef POPC_LOGGER_H
#define POPC_LOGGER_H

#include <stdarg.h>
#include <sstream>

enum LOGLEVEL {
    __DEBUG__,
    //__DEV__, 
    __INFO__,
    __CORE__, 
    __WARNING__,
    __ERROR__, 
    __LAST__  // Only for dimensioning array
};

#define LOG_GENERIC(_log_level, _log_msg) {\
    std::stringstream ss;\
    ss<<_log_msg;\
    popc_logger(_log_level, __FILE__, __LINE__, __FUNCTION__, ss.str().c_str());\
}

#define LOG_DEBUG(_log_msg, ...)   popc_logger(__DEBUG__,_log_msg, ##__VA_ARGS__)
#define LOG_INFO(_log_msg, ...)    popc_logger(__INFO__,_log_msg, ##__VA_ARGS__)
#define LOG_CORE(_log_msg, ...)    popc_logger(__CORE__,_log_msg, ##__VA_ARGS__)
#define LOG_WARNING(_log_msg, ...) popc_logger(__WARNING__,_log_msg, ##__VA_ARGS__)
#define LOG_ERROR(_log_msg, ...)   popc_logger(__ERROR__,_log_msg, ##__VA_ARGS__)

#define LOG_DEBUG_IF(_cond,_log_msg, ...) if(_cond){popc_logger(__DEBUG__,_log_msg, ##__VA_ARGS__)}

int popc_logger(LOGLEVEL level, const char *format,...);
int popc_logger(LOGLEVEL level, const char* file, int line, const char* function, const char *format);

#endif /* POPC_LOGGER */
