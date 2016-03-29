#ifndef APP_MICROBILL_MYSQL_CLIENT_H
#define APP_MICROBILL_MYSQL_CLIENT_H

#include <mysql/mysql.h>
#include "db_helper.h"

namespace microbill {

class MysqlClient : public DBClient {
public:
	MysqlClient() : _host("127.0.0.1"),
			_user("micro"),
			_passwd("bill"),
			_db("microbill"),
			_port(3306),
			_client_flag(0) {}
	virtual ~MysqlClient() {}

	bool init();
	bool init(std::string table_name);
	bool truncate();
	void close();

	bool query_records(const std::vector<std::string>& ids, std::vector<RecordContent>* record_contents);

	bool push_records(const std::vector<RecordContent>& new_record_contents, const std::vector<ModifyRecordPair>& modify_records);

	bool query_single_record(const std::string& id, RecordContent* content);

private:
	MYSQL _mysql;
	std::string _host;
	std::string _user;
	std::string _passwd;
	std::string _db;
	unsigned int _port;
	std::string _unix_socket;
	unsigned long _client_flag = 0;

	std::string _table;
	const std::string _query_records_sql_prefix = "select id, year, month, day, pay_earn, gay, comments, cost, is_deleted from ";
	const std::string _insert_records_sql_snippet = " (id, year, month, day, pay_earn, gay, comments, cost, is_deleted) VALUES ";
};

}

#endif

