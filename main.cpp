#include "config.h"

int main(int argc,char *argv[])
{
    string user = "root";
    string passwd = "333";
    string dbname = "webdb";

    config c;
    c.parse_config(argc,argv);

    webServer server;
    server.init(c.port,user,passwd,dbname,c.logWrite,c.OPT_LINGER,c.TRIGMode,c.sql_num,c.thread_num,c.close_log,c.actor_model);
    server.log_write();
    server.sql_pool();
    server.threadPool();
    server.trig_model();
    server.eventListen();
    server.eventLoop();
    return 0;
}