#include "config.h"

config::config()
{
    port = 9096;
    logWrite = 0;
    TRIGMode = 0;
    LISTENTrigmode = 0;
    CONNTrigmode = 0;
    OPT_LINGER = 0;
    sql_num = 8;
    thread_num = 1;
    close_log = 0;
    actor_model = 0;
}

config::~config()
{
    
}

void config::parse_config(int argc,char *argv[])
{
    int opt;
    const char *str = "p:l:m:o:s:t:c:a";
    while((opt = getopt(argc,argv,str)) != -1)
    {
        switch (opt)
        {
        case 'p':
            port = atoi(optarg);
            break;
        case 'l':
            logWrite = atoi(optarg);
            break;
        case 'm':
            TRIGMode = atoi(optarg);
            break;
        case 'o':
            OPT_LINGER = atoi(optarg);
            break;
        case 's':
            sql_num = atoi(optarg);
            break;
        case 't':
            thread_num = atoi(optarg);
            break;
        case 'c':
            close_log = atoi(optarg);
            break;
        case 'a':
            actor_model = atoi(optarg);
            break;
        default:
            break;
        }
    }
}