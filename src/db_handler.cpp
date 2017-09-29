
#include "db_handler.h"

namespace microbill {

bool DBHandler::init() {
    return true;
}

bool DBHandler::push_records(const SQLs& sqls) {
    for (SQL sql : sqls) {
        if (!_client->query(sql, NULL)) {
            return false;
        }
    }
    return true;
}

bool DBHandler::pull_records(const SQLs& sqls, RecordLines* record_lines) {
    for (SQL sql : sqls) {
        if (!_client->query(sql, record_lines)) {
            return false;
        }
    }
    return true;
}

bool DBHandler::truncate(const std::string& table_name) {
    SQL sql = std::make_pair(SQLType::UPDATE, _truncate_sql + table_name);
    return _client->query(sql, NULL);
}

}
