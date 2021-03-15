# Handy C++ logger (`ict::logger`)

`ict::logger` is a handy logger for C++ applications. Its interface is based on `std::ostream`.

## Basic usage

```c
#include <libict/logger/logger.hpp>
int main(int argc,const char **argv){
    LOGGER_THREAD;
    LOGGER_BASEDIR; 
    LOGGER_SET(std::cerr);// If set then output is sent to std::cerr
    LOGGER_SET("test"); // If set then output is sent to syslog and tagged with 'test'.

    // Logs one line with critical severity
    LOGGER_CRIT<<__LOGGER__<<"Test string ... "<<std::endl;
    // Logs one line with error severity
    LOGGER_ERR<<__LOGGER__<<"Test string ... "<<std::endl;
    // Logs one line with warning severity
    LOGGER_WARN<<__LOGGER__<<"Test string ... "<<std::endl;
    // Logs one line with notice severity
    LOGGER_NOTICE<<__LOGGER__<<"Test string ... "<<std::endl;
    // Logs one line with info severity
    LOGGER_INFO<<__LOGGER__<<"Test string ... "<<std::endl;
    // Logs one line with debug severity
    LOGGER_DEBUG<<__LOGGER__<<"Test string ... "<<std::endl;
}
```

* Macro `LOGGER_THREAD;` provides logger declaration for given thread. It should be declared for each thread (if the logger should be available there). Declaration `LOGGER_THREAD;` should exist for the entire duration of the thread, so the best place is the beginig of main thread function. If not set, then no logs are printed from this thread.
* `LOGGER_BASEDIR;` is a macro that saves path of current file. Every path from `__LOGGER__` macro is printed as relative to this path. It should be declared only once for a whole application. If not set, then absolute paths are printed.
* `LOGGER_SET(stream)` is a configuration of logger output. It should point out the stream (`std::ostream` type) where logs are to be printed. It is allowed to configure more than one output. If none is configured, then no logs are printed. If a string is provided instead of a stream parameter, then syslog is used as output (a string parameter provides a tag to be used in each log line).
* Macro `__LOGGER__` prints info about place in the code (file name, file line and function name).
* `LOGGER_CRIT`, `LOGGER_ERR`, `LOGGER_WARN`, `LOGGER_NOTICE`, `LOGGER_INFO` and `LOGGER_DEBUG` provides `std::ostream` objects that can be used for logging. Each `std::endl` put to these streams prints one line in the log.

Example output:
```
2021-01-14 19:07:24(+0100) CRITICAL logger.cpp:684 (int test_tc1()) Test string ... 
2021-01-14 19:07:24(+0100) ERROR logger.cpp:685 (int test_tc1()) Test string ...
2021-01-14 19:07:24(+0100) WARNING logger.cpp:686 (int test_tc1()) Test string  ...
2021-01-14 19:07:24(+0100) NOTICE logger.cpp:687 (int test_tc1()) Test string ...
2021-01-14 19:07:24(+0100) INFO logger.cpp:688 (int test_tc1()) Test string ...
2021-01-14 19:07:24(+0100) DEBUG logger.cpp:689 (int test_tc1()) Test string ...
```

## Enabling/disabling at compilation time

The selected severities of logger can be permanently disabled at compilation time. It is done by including one of following files:
* `disable-crit.hpp` - disables critical severity;
* `disable-err.hpp` - disables error severity;
* `disable-warn.hpp` - disables warning severity;
* `disable-notice.hpp` - disables notice severity;
* `disable-info.hpp` - disables info severity;
* `disable-debug.hpp` - disables debug severity;
* `disable-all.hpp` - disables entire logger.

If only in a part of the file the logger should be disabled, then selected severities of the logger can be enabled, by one of following files:
* `enable-crit.hpp` - enables critical severity;
* `enable-err.hpp` - enables error severity;
* `enable-warn.hpp` - enables warning severity;
* `enable-notice.hpp` - enables notice severity;
* `enable-info.hpp` - enables info severity;
* `enable-debug.hpp` - enables debug severity;
* `enable-all.hpp` - enables entire logger.

Example:
```c
LOGGER_CRIT<<__LOGGER__<<"This line is enalbled ... "<<std::endl;
LOGGER_DEBUG<<__LOGGER__<<"This line is enalbled ... "<<std::endl;
#include <libict/logger/disable-debug.hpp>
LOGGER_CRIT<<__LOGGER__<<"This line is enalbled ... "<<std::endl;
LOGGER_DEBUG<<__LOGGER__<<"This line is disabled ... "<<std::endl;
#include <libict/logger/enable-debug.hpp>
LOGGER_CRIT<<__LOGGER__<<"This line is enalbled ... "<<std::endl;
LOGGER_DEBUG<<__LOGGER__<<"This line is enalbled ... "<<std::endl;
```

## Log filtering

The output of the logger can be filtered. Proper filter can be set by the second parameter of `LOGGER_SET(stream,filter)` macro. Default value is `ict::logger::all` - all severities. If `ict::logger::none` is used, then given output is disabled.

Following values are allowed:
* `ict::logger::critical` - critical severity only;
* `ict::logger::error` - error severity only;
* `ict::logger::warning` - warning severity only;
* `ict::logger::notice` - notice severity only;
* `ict::logger::info` - info severity only;
* `ict::logger::debug` - debug severity only;
* `ict::logger::errors` - critical and error severity only;
* `ict::logger::warnings` - critical and error and warning severity only;
* `ict::logger::notices` - critical and error and warning and notice severity only;
* `ict::logger::infos` - critical and error and warning and notice and info severity only;
* `ict::logger::all` - all severities;
* `ict::logger::nocritical` - error and warning and notice and info and debug severity only;
* `ict::logger::noerrors` - warning and notice and info and debug severity only;
* `ict::logger::nowarnings` - notice and info and debug severity only;
* `ict::logger::nonotices` - info and debug severity only;
* `ict::logger::nodebug` - critical and error and warning and notice and info severity only;
* `ict::logger::none` - disables given output.

```c
#include <libict/logger/logger.hpp>
int main(int argc,const char **argv){
    LOGGER_THREAD;
    LOGGER_BASEDIR; 
    LOGGER_SET(std::cerr);// All severities are sent to std::cerr
    LOGGER_SET("test",ict::logger::errors); // Only critical and error severity is printed to syslog
}
```

## Advanced usage

Some times there is no need to print detailed logs when everything is OK. The need is only if error happens. In such case buffered logging can be used. Lines with low severities (info and debug) are buffered. If no lines with errors (critical and error severity) are printed, then buffer is cleared, otherwise all lines from buffer are printed.

Buffered logging is declared by `LOGGER_LAYER;` macro. It should be placed at the begining of variable sope where buffered logging is to be enabled.

Example:
```c
int thread(){
    LOGGER_THREAD;
    
    {//Scope where buffered logging is enabled
        LOGGER_LAYER;
        // This log is printed
        LOGGER_WARN<<__LOGGER__<<"Test string ... "<<std::endl;
        // This log is printed
        LOGGER_NOTICE<<__LOGGER__<<"Test string ... "<<std::endl;
        // This log is buffered
        LOGGER_INFO<<__LOGGER__<<"Test string ... "<<std::endl;
        // This log is buffered
        LOGGER_DEBUG<<__LOGGER__<<"Test string ... "<<std::endl;
        // Buffer is cleared because no error occured
    }

    {//Scope where buffered logging is enabled
        LOGGER_LAYER;
        // This log is printed
        LOGGER_WARN<<__LOGGER__<<"Test string ... "<<std::endl;
        // This log is printed
        LOGGER_NOTICE<<__LOGGER__<<"Test string ... "<<std::endl;
        // This log is buffered
        LOGGER_INFO<<__LOGGER__<<"Test string ... "<<std::endl;
        // This log is buffered
        LOGGER_DEBUG<<__LOGGER__<<"Test string ... "<<std::endl;
        // This log is printed
        LOGGER_ERR<<__LOGGER__<<"Test string ... "<<std::endl;
        // Buffer is printed because an error occured
    }
}
```

Lines in the log that was printed from a buffer are marked with `|` (pipe) character (before severity string).

Example output:
```
2021-01-14 19:17:34(+0100) | DEBUG logger.cpp:689 (int test_tc1()) Test string ...
```
