
#define LOGGER_INCLUDED

#ifndef LOGGER_INCLUDED

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

class Logger{

private:
    char _function[100];
    int _indent;

    static int indent;

    static void log(int indent, const char *fmt, va_list args){
        char buffer[1000];
        char indentation[1000];
        vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
        indentation[0] = '\0';
        for (int i = 0; i < indent; i++)
        {
            strncat(indentation, "  ", sizeof(indentation) - 1);
        }

        Serial.printf("%s%s\n", indentation, buffer);
    }

    static bool accept(const char *function ){
        const char *disabled[] = {
            "insideBox",  "timerTick"
        };

        for( int i = 0 ; i < sizeof(disabled)/sizeof(*disabled) ; i++ ){
            if( !strcmp(function,disabled[i]) ){
                return false;
            }
        }
        return true;
    }

public:

    Logger(){
        Logger("");
    }

    Logger(const char* function){
        strncpy(_function, function, sizeof(_function) - 1);
        if( !Logger::accept(_function) ){
            return;
        }
        _indent = Logger::indent;

        operator()("--> %s", _function);
        Logger::indent += 1;
        _indent = Logger::indent;

    }

    void operator()(const char* fmt, ... ){
        if (!Logger::accept(_function))
        {
            return;
        }
        va_list args;
        va_start(args, fmt);
        log(_indent,fmt,args);
        va_end(args);
    }

    ~Logger(){
        if (!Logger::accept(_function))
        {
            return;
        }
        Logger::indent -= 1;
        _indent = Logger::indent;
        operator()("<-- %s", _function);
    }

 
};

int Logger::indent = 0;

#endif
