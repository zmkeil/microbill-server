#include <iostream>
#include <sstream>
#include <comlog/info_log_context.h>
#include "mysql_client.h"

namespace microbill {

static bool set_record_contents(const char* fp, unsigned long len, MYSQL_FIELD* fields, int index, RecordContent& record_content)
{
	(void) len;
	(void) fields;
	switch (index) {
		case 0:
			// check fields[index] == "id" ?
			record_content.id = fp;
			break;
		case 1:
			record_content.year = atoi(fp);
			break;
		case 2:
			record_content.month = atoi(fp);
			break;
		case 3:
			record_content.day = atoi(fp);
			break;
		case 4:
			record_content.pay_earn = atoi(fp);
			break;
		case 5:
			record_content.gay = fp;
			break;
		case 6:
			record_content.comments = fp;
			break;
		case 7:
			record_content.cost = atoi(fp);
		case 8:
			record_content.is_deleted = atoi(fp);
			break;
		default:
			return false;
			break;
	}
	return true;
}

bool MysqlClient::init(const MysqlOptions& options)
{
	_host = options.host();
	_port = options.port();
	_db = options.db();
	_user = options.user();
	_passwd = options.passwd();
	_table = options.table();
	_client_flag = options.flag();

	mysql_init(&_mysql);
	mysql_options(&_mysql, MYSQL_OPT_RECONNECT, "true");
	if (!mysql_real_connect(&_mysql, _host.c_str(), _user.c_str(), _passwd.c_str(),
				_db.c_str(), _port, NULL, _client_flag)) {
		LOG(ERROR, "failed to connect to database: Error: %s\n", mysql_error(&_mysql));
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

bool MysqlClient::truncate()
{
	std::string sql = "truncate table " + _table;
	int ret = mysql_real_query(&_mysql, sql.c_str(), sql.size());
	if (ret) {
		LOG(ERROR, "Failed to truncate table %s, Error: %s", _table.c_str(), mysql_error(&_mysql));
		return false;
	}
	return true;
}

void MysqlClient::close()
{
	return mysql_close(&_mysql);
}

bool MysqlClient::query_single_record(const std::string& id, RecordContent* content)
{
	std::string sql = _query_records_sql_prefix + _table + " where id = '" + id + "'";
	int ret = mysql_real_query(&_mysql, sql.c_str(), sql.size());
	if (ret) {
		LOG(ERROR, "Failed to query records, sql \"%s\", Error: %s", sql.c_str(), mysql_error(&_mysql));
		return false;
	}

	MYSQL_RES *results = mysql_store_result(&_mysql);
	MYSQL_ROW row = mysql_fetch_row(results);
	if (!row) {
		LOG(NOTICE, "No record of id = \"%s\"", id.c_str());
		return false;
	}

	unsigned int num_fields = mysql_num_fields(results);
	MYSQL_FIELD  *fields = mysql_fetch_fields(results);
	unsigned long* length = mysql_fetch_lengths(results);
	for (int i = 0;i < (int)num_fields;++i) {
		if (!set_record_contents(row[i], length[i], fields, i, *content)) {
			LOG(NOTICE, "Resolve the %dth field of row(\"%s\") failed", i, row[i]);
		}
	}
	mysql_free_result(results);
	if(mysql_errno(&_mysql)) {
		LOG(ERROR, "fetch row error: %s", mysql_error(&_mysql));
		return false;
	}
	return true;
}

bool MysqlClient::query_records(const std::vector<std::string>& ids, std::vector<RecordContent>* record_contents)
{
	std::string id_str("(");
	for (std::string id : ids) {
		id_str += "'";
		id_str += id;
		id_str += "',";
	}
	std::string sql =  _query_records_sql_prefix + _table + " where id in " + id_str;
	sql[sql.size() - 1] = ')';
	int ret = mysql_real_query(&_mysql, sql.c_str(), sql.size());
	if (ret) {
		LOG(ERROR, "Failed to query records, sql \"%s\", Error: %s", sql.c_str(), mysql_error(&_mysql));
		return false;
	}

	MYSQL_RES *results = mysql_store_result(&_mysql);
	unsigned int num_fields = mysql_num_fields(results);
	MYSQL_FIELD  *fields = mysql_fetch_fields(results);
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(results))) {
		unsigned long* length = mysql_fetch_lengths(results);
		RecordContent record_content;
		for (int i = 0;i < (int)num_fields;++i) {
			if (!set_record_contents(row[i], length[i], fields, i, record_content)) {
				LOG(NOTICE, "Resolve the %dth field of row(\"%s\") failed", i, row[i]);
			}
		}
		// Here we don't guess the order of results returned by DB
		// later, we build a simple map for index
		record_contents->push_back(record_content);
	}
	mysql_free_result(results);
	if(mysql_errno(&_mysql)) {
		LOG(ERROR, "fetch row error: %s", mysql_error(&_mysql));
		return false;
	}
	return true;
}

static std::string content2string(const RecordContent& content)
{
	std::stringstream ss;
	std::string str;

	ss << "('" << content.id << "'," << content.year << "," << content.month << ","
		<< content.day << "," << content.pay_earn << ",";
	ss >> str;
	str += "'";
	str += content.gay;
	str += "','";
	str += content.comments;
	str += "',";

	std::stringstream ss1;
	std::string str1;
	ss1 << content.cost << "," << content.is_deleted << "),";
	ss1 >> str1;

	str += str1;
	//std::cout << str << std::endl;
	return str;
}

bool MysqlClient::push_records(const std::vector<RecordContent>& new_record_contents, const std::vector<ModifyRecordPair>& modify_records)
{
	bool ret = true;
	// insert new records
	std::string sql = "insert into " + _table + _insert_records_sql_snippet;
	if (new_record_contents.size() > 0) {
		for (const RecordContent& content : new_record_contents) {
			sql += content2string(content);
		}
		sql[sql.size() - 1] = ' ';
		if (mysql_real_query(&_mysql, sql.c_str(), sql.size())) {
			char error_buf[1024];
			snprintf(error_buf, 1024, "%s", mysql_error(&_mysql));
			if (strstr(error_buf, "Duplicate entry")) {
				LOG(ERROR, "some entry already inserted, we will than insert one by one to avoid them");
				// for duplicate, we ignore it because client will retry conservatively
				// then we insert the records one by one, any other method ?
				for (const RecordContent& content : new_record_contents) {
					std::string sql_single = "insert into " + _table + _insert_records_sql_snippet + content2string(content);
					sql_single[sql_single.size() - 1] = ' ';
					if (mysql_real_query(&_mysql, sql_single.c_str(), sql_single.size())) {
						memset(error_buf, 0, 1024);
						snprintf(error_buf, 1024, "%s", mysql_error(&_mysql));
						if (strstr(error_buf, "Duplicate entry")) {
							LOG(WARN, "%s alreay inserted before", content.id.c_str());
							continue;
						} else {
							ret = false;
							LOG(ERROR, "Failed to insert records, sql \"%s\", Error: %s", sql_single.c_str(), mysql_error(&_mysql));
							continue;	// also continue to insert the followed records
						}
					}
				}
			} else {
				// other DB error
				LOG(ERROR, "Failed to insert records, sql \"%s\", Error: %s", sql.c_str(), mysql_error(&_mysql));
				ret = false;	
			}
		}
	}

	// update records
	std::string update_snippet = "update " + _table + " SET ";
	sql = update_snippet;
	for (const ModifyRecordPair& record_pair : modify_records) {
		if (record_pair.first == "id") {
			sql[sql.size() - 1] = ' ';
			sql += "where id = ";
			sql += record_pair.second;
			LOG(ERROR, "update sql \"%s\"", sql.c_str());
			if (mysql_real_query(&_mysql, sql.c_str(), sql.size())) {
				LOG(ERROR, "Failed to update records, sql \"%s\", Error: %s", sql.c_str(), mysql_error(&_mysql));
				ret = false;
			}
			sql = update_snippet;
		} else {
			sql += record_pair.first;
			sql += "=";
			sql += record_pair.second;
			sql += ",";
		}
	}

	return ret;
}

}
