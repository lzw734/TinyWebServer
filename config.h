#ifndef CONFIG_H
#define CONFIG_H
#include "http_server.h"

class config
{
public:
    config();
    ~config();

    void parse_config(int argc,char *argv[]);
    int port;
    int logWrite;
    int TRIGMode;
    int LISTENTrigmode;
    int CONNTrigmode;
    int OPT_LINGER;
    int sql_num;
    int thread_num;
    int close_log;
    int actor_model;
};

#endif