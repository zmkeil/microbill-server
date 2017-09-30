#include <iostream>
#include <comlog/info_log_context.h>
#include "msg_adaptor.h"
#include "mysql_client.h"

namespace microbill {

static void set_record_line(MYSQL_ROW row, MYSQL_FIELD* fields,
        unsigned int num_fields, RecordLine* record_line) {
    for (unsigned int i = 0; i < num_fields; ++i) {
        if (!row[i]) {
            continue;
        }
        record_line->insert(std::pair<std::string, std::string>(fields[i].name, row[i]));
    }
}

bool MysqlClient::init(const MysqlOptions& options)
{
	_host = options.host();
	_port = options.port();
	_db = options.db();
	_user = options.user();
	_passwd = options.passwd();
	_client_flag = options.flag();

	mysql_init(&_mysql);
	mysql_options(&_mysql, MYSQL_OPT_RECONNECT, "true");
	if (!mysql_real_connect(&_mysql, _host.c_str(), _user.c_str(), _passwd.c_str(),
				_db.c_str(), _port, NULL, _client_flag)) {
		LOG(ERROR, "failed to connect to database: Error: %s", mysql_error(&_mysql));
		return false;
	}
	if (mysql_options(&_mysql, MYSQL_SET_CHARSET_NAME, "utf8")) {
		LOG(ERROR, "failed to set charset, the current charset is %s", mysql_character_set_name(&_mysql));
		return false;
	}
	if (mysql_set_character_set(&_mysql, "utf8")) {
		LOG(ERROR, "failed to set charset, the current charset is %s", mysql_character_set_name(&_mysql));
		return false;
	}
	return true;
}

void MysqlClient::close()
{
	return mysql_close(&_mysql);
}

bool MysqlClient::query(const SQL& sql, RecordLines* record_lines)
{
	bool ret = true;
    SQLType sql_type = sql.first;
    std::string sql_str = sql.second;

	// insert new records
    if (sql_type == SQLType::INSERT) {
		if (mysql_real_query(&_mysql, sql_str.c_str(), sql_str.size())) {
			char error_buf[1024] = {0};
			snprintf(error_buf, 1024, "%s", mysql_error(&_mysql));
			if (strstr(error_buf, "Duplicate entry")) {
				LOG(WARN, "entry already inserted [%s]", sql_str.c_str());
				// for duplicate, we ignore it because client will retry conservatively
			} else {
                ret = false;
                LOG(ERROR, "entry insert failed [%s], Error [%s]", sql_str.c_str(), error_buf);
            }
		}
	}

	// update records
    else if (sql_type == SQLType::UPDATE) {
		if (mysql_real_query(&_mysql, sql_str.c_str(), sql_str.size())) {
            LOG(ERROR, "entry update failed [%s], Error [%s]", sql_str.c_str(), mysql_error(&_mysql));
			ret = false;
		}
	}

    // select records
    else if (sql_type == SQLType::SELECT) {
        if (!record_lines) {
            LOG(ERROR, "record_lines is null when select");
            return false;
        }
	    if (mysql_real_query(&_mysql, sql_str.c_str(), sql_str.size())) {
            LOG(ERROR, "entry select failed [%s], Error [%s]", sql_str.c_str(), mysql_error(&_mysql));
	    	return false;
	    }
	    MYSQL_RES *results = mysql_store_result(&_mysql);
	    unsigned int num_fields = mysql_num_fields(results);
	    MYSQL_FIELD  *fields = mysql_fetch_fields(results);
	    MYSQL_ROW row;
	    while ((row = mysql_fetch_row(results))) {
	    	RecordLine record_line;
            set_record_line(row, fields, num_fields, &record_line);
            record_lines->push_back(record_line);
	    }
	    mysql_free_result(results);
	    if(mysql_errno(&_mysql)) {
	    	LOG(ERROR, "fetch row error: %s", mysql_error(&_mysql));
	    	return false;
	    }
    }
	return ret;
}

}
