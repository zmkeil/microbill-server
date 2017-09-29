#ifndef APP_MICROBILL_DB_HANDLER_H
#define APP_MICROBILL_DB_HANDLER_H

#include "msg_adaptor.h"
#include "db_client.h"

namespace microbill {

class DBHandler {
public:
	DBHandler(DBClient* client) : _client(client) {}
	virtual ~DBHandler() {}

	bool init();
	bool push_records(const SQLs& sqls);
	bool pull_records(const SQLs& sqls, RecordLines* record_lines);

	bool truncate(const std::string& table_name);

private:
	DBClient* _client;

private:
    const std::string _truncate_sql = "truncate table ";
};

}

#endif
