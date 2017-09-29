#ifndef APP_MICROBILL_MYSQL_CLIENT_H
#define APP_MICROBILL_MYSQL_CLIENT_H

#include <mysql/mysql.h>
#include "db_client.h"
#include "bill_config.pb.h"

namespace microbill {

class MysqlClient : public DBClient {
public:
	MysqlClient() {}
	virtual ~MysqlClient() {}

	bool init(const MysqlOptions& mysql_options);
	void close();
    bool query(const SQL& sql, RecordLines* record_lines);

private:
	MYSQL _mysql;
	std::string _host;
	std::string _user;
	std::string _passwd;
	std::string _db;
	unsigned int _port;
	std::string _unix_socket;
	unsigned long _client_flag = 0;
};

}

#endif

