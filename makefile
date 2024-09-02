CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2
endif

server: main.cpp  ./timer/lst_timer.cpp ./http_conn/http_conn.cpp ./log/log.cpp ./cgimysql/mysql_conn_pool.cpp  http_server.cpp config.cpp
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient

clean:
	rm  -r serve