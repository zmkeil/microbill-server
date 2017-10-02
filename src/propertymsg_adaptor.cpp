
#include <iostream>
#include <string>
#include <comlog/info_log_context.h>
#include "propertymsg_adaptor.h"

namespace microbill {

std::string PropertyMsgAdaptor::s_pocket_table_name = "unkown";
std::string PropertyMsgAdaptor::s_assets_table_name = "unkown";

static std::string pocketr2sql_insert(const PocketRecord& pocket_record) {
    std::stringstream ss;
    ss << "('" << pocket_record.sid() << "'," << pocket_record.year() << "," << pocket_record.month() << ",'"
        << pocket_record.comments() << "'," << pocket_record.money() << "," << pocket_record.is_deleted() << ")";
    std::string str = ss.str();
    return str;
}

static std::string assetsr2sql_insert(const AssetsRecord& assets_record) {
    std::stringstream ss;
    ss << "('" << assets_record.sid() << "'," << assets_record.year() << "," << assets_record.month() << ","
        << assets_record.day() << "," << assets_record.store_addr() << "," << assets_record.flow_type() << ","
        << assets_record.money() << "," << assets_record.store_addr_op() << "," << assets_record.is_deleted() << ")";
    std::string str = ss.str();
    return str;
}

static std::string pocketr2sql_update(const PocketRecord& pocket_record) {
    std::stringstream ss;

	if (pocket_record.has_money()) {
        ss << "cost=" << pocket_record.money() << ",";
	}
	if (pocket_record.has_is_deleted()) {
        ss << "is_deleted=" << (pocket_record.is_deleted() ? "1" : "0") << ",";
	}

    std::string str = ss.str();
    str[str.length() - 1] = ' ';
    str += "where sid='" + pocket_record.sid() + "'";
	return str;
}

static std::string assetsr2sql_update(const AssetsRecord& assets_record) {
    std::stringstream ss;

	if (assets_record.has_day()) {
		ss << "day=" << assets_record.day() << ",";
	}
	if (assets_record.has_store_addr()) {
        ss << "store_addr=" << assets_record.store_addr() << ",";
	}
	if (assets_record.has_flow_type()) {
        ss << "flow_type=" << assets_record.flow_type() << ",";
	}
	if (assets_record.has_money()) {
        ss << "money=" << assets_record.money() << ",";
	}
	if (assets_record.has_store_addr_op()) {
        ss << "store_addr_op=" << assets_record.store_addr_op() << ",";
	}
	if (assets_record.has_is_deleted()) {
        ss << "is_deleted=" << (assets_record.is_deleted() ? "1" : "0") << ",";
	}

    std::string str = ss.str();
    str[str.length() - 1] = ' ';
    str += "where sid='" + assets_record.sid() + "'";
	return str;
}


void PropertyMsgAdaptor::push_sqls(EventLines* event_lines, SQLs* sqls) {
    if (!_push_property_records || _push_property_records->size() == 0) {
        return;
    }
    EventLine event_line;
    for (auto property_record : *_push_property_records) {
        // check has action_type and property_type
        if (!property_record.has_type() || !property_record.has_property_type()) {
            continue;
        }
        PropertyRecord_Type action_type = property_record.type();
        PropertyRecord_PropertyType property_type = property_record.property_type();

        std::string sid;
        // pocket
        if (property_type == PropertyRecord::POCKET_MONEY) {
            if (!property_record.has_pocket_record()) {
                continue;
            }
            sid = property_record.pocket_record().sid();
            // new insert
            if (action_type == PropertyRecord::NEW) {
                std::string sql_str = "insert " + s_pocket_table_name + _insert_pocket_sql_snippet
                        + pocketr2sql_insert(property_record.pocket_record());
                SQL sql = std::make_pair(SQLType::INSERT, sql_str);
                sqls->push_back(sql);
                _push_new_ids += "p_" + sid + "|";
            }
            // old update
            else if (action_type == PropertyRecord::UPDATE) {
                std::string sql_str = "update " + s_pocket_table_name + " set " 
                        + pocketr2sql_update(property_record.pocket_record());
                SQL sql = std::make_pair(SQLType::UPDATE, sql_str);
                sqls->push_back(sql);
                _push_update_ids += "p_" + sid + "|";
            }
        }
        // assets
        else if (property_type == PropertyRecord::FIXED_ASSETS) {
            if (!property_record.has_assets_record()) {
                continue;
            }
            sid = property_record.assets_record().sid();
            // new insert
            if (action_type == PropertyRecord::NEW) {
                std::string sql_str = "insert " + s_assets_table_name + _insert_assets_sql_snippet
                        + assetsr2sql_insert(property_record.assets_record());
                SQL sql = std::make_pair(SQLType::INSERT, sql_str);
                sqls->push_back(sql);
                _push_new_ids += "a_" + sid + "|";
            }
            // old update
            else if (action_type == PropertyRecord::UPDATE) {
                std::string sql_str = "update " + s_assets_table_name + " set " 
                        + assetsr2sql_update(property_record.assets_record());
                SQL sql = std::make_pair(SQLType::UPDATE, sql_str);
                sqls->push_back(sql);
                _push_update_ids += "a_" + sid + "|";
            }
        } else {
            return;
        }

        // event
        event_line.clear();
        event_line.push_back(action_type == PropertyRecord::NEW ? "0" : "1");
        event_line.push_back(property_type == PropertyRecord::POCKET_MONEY ? "P" : "A");
        event_line.push_back(sid);
        event_lines->push_back(event_line);
    }
}

void PropertyMsgAdaptor::pull_sqls(const EventLines& event_lines, SQLs* sqls) {
    std::string in_pocket_ids;
    std::string in_assets_ids;
    for (auto event_line : event_lines) {
        // format: action_type id
        if (event_line.size() != 3) {
            continue;
        }
        if (event_line[1] == "P") {
            _pull_ids += "p_" + event_line[1] + "|";
            in_pocket_ids += "'" + event_line[2] + "',";
        } else if (event_line[1] == "P") {
            _pull_ids += "a_" + event_line[1] + "|";
            in_assets_ids += "'" + event_line[2] + "',";
        }
    }
    if (in_pocket_ids.size() > 0) {
        in_pocket_ids[in_pocket_ids.length() - 1] = ')';
        std::string sql_str = _query_pocket_sql_prefix
                + s_pocket_table_name + " where id in (" + in_pocket_ids;
        SQL sql = std::make_pair(SQLType::SELECT, sql_str);
        sqls->push_back(sql);
    }
    if (in_assets_ids.size() > 0) {
        in_assets_ids[in_assets_ids.length() - 1] = ')';
        std::string sql_str = _query_assets_sql_prefix
                + s_assets_table_name + " where id in (" + in_assets_ids;
        SQL sql = std::make_pair(SQLType::SELECT, sql_str);
        sqls->push_back(sql);
    }
}

void PropertyMsgAdaptor::set_pull_records(const EventLines& event_lines,
        const RecordLines& record_lines) {
    if (!_pull_property_records) {
        return;
    }
    std::map<std::string/*sid*/, Record_Type/*action*/> action_map;
    for (auto event_line : event_lines) {
        if (action_map.find(event_line[1]) != action_map.end()) {
            // already exist, only can be NEW - UPDATE - UPDATE ...
            // so just use NEW
            continue;
        }
        action_map[event_line[1]] = (event_line[0] == "0" ? Record::NEW : Record::UPDATE);
    }
/*    for (auto record_line : record_lines) {
        Record* record = _pull_property_records->Add();
        if (record_line["id"] == "" ||
                record_line["year"] == "" ||
                record_line["month"] == "" ||
                record_line["day"] == "" ||
                record_line["pay_earn"] == "" ||
                record_line["gay"] == "" ||
                record_line["comments"] == "" ||
                record_line["cost"] == "" ||
                record_line["is_deleted"] == "") {
            LOG(WARN, "bad property record sid = %s", record_line["id"].c_str());
            continue;
        }
        std::string id = record_line["id"];
        record->set_id(id);
        if (action_map.find(id) != action_map.end()) {
            record->set_type(action_map[id]);
        } else {
            // never happen
            record->set_type(Record::NEW);
        }
        record->set_year(atoi(record_line["year"].c_str()));
        record->set_month(atoi(record_line["month"].c_str()));
        record->set_day(atoi(record_line["day"].c_str()));
        record->set_pay_earn(atoi(record_line["pay_earn"].c_str()));
        record->set_gay(record_line["gay"]);
        record->set_comments(record_line["comments"]);
        record->set_cost(atoi(record_line["cost"].c_str()));
        record->set_is_deleted(atoi(record_line["is_deleted"].c_str()));
    }
*/
}

}

