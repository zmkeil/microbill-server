
#include <iostream>
#include <string>
#include "billmsg_adaptor.h"

namespace microbill {

std::string BillMsgAdaptor::s_bill_table_name = "unkown";

static std::string billr2sql_insert(const Record& bill_record) {
    std::stringstream ss;
    ss << "('" << bill_record.id() << "'," << bill_record.year() << "," << bill_record.month() << ","
        << bill_record.day() << "," << bill_record.pay_earn() << ",'" << bill_record.gay() << "','"
        << bill_record.comments() << "'," << bill_record.cost() << "," << bill_record.is_deleted() << ")";
    std::string str = ss.str();
    return str;
}

static std::string billr2sql_update(const Record& bill_record) {
    std::stringstream ss;

	if (bill_record.has_day()) {
		ss << "day=" << bill_record.day() << ",";
	}
	if (bill_record.has_pay_earn()) {
        ss << "pay_earn=" << (bill_record.pay_earn() ? "1" : "0") << ",";
	}
	if (bill_record.has_gay()) {
        ss << "gay='" << bill_record.gay() << "',";
	}
	if (bill_record.has_comments()) {
        ss << "comments='" << bill_record.comments() << "',";
	}
	if (bill_record.has_cost()) {
        ss << "cost=" << bill_record.cost() << ",";
	}
	if (bill_record.has_is_deleted()) {
        ss << "is_deleted=" << (bill_record.is_deleted() ? "1" : "0") << ",";
	}

    std::string str = ss.str();
    str[str.length() - 1] = ' ';
    str += "where id='" + bill_record.id() + "'";
	return str;
}

void BillMsgAdaptor::push_sqls(EventLines* event_lines, SQLs* sqls) {
    EventLine event_line;
    for (auto bill_record : _push_bill_records) {
        // check has sid
        if (!bill_record.has_id()) {
            continue;
        }

        // new insert
        if (bill_record.type() == Record::NEW) {
            std::string sql_str = "insert " + s_bill_table_name
                    + _insert_records_sql_snippet + billr2sql_insert(bill_record);
            SQL sql = std::make_pair(SQLType::INSERT, sql_str);
            sqls->push_back(sql);
            _push_new_ids += bill_record.id() + "|";
        }
        // old update
        else if (bill_record.type() == Record::UPDATE) {
            std::string sql_str = "update " + s_bill_table_name
                    + " set " + billr2sql_update(bill_record);
            SQL sql = std::make_pair(SQLType::UPDATE, sql_str);
            sqls->push_back(sql);
            _push_update_ids += bill_record.id() + "|";
        }

        // event
        event_line.clear();
        event_line.push_back(bill_record.type() == Record::NEW ? "0" : "1");
        event_line.push_back(bill_record.id());
        event_lines->push_back(event_line);
    }
}

void BillMsgAdaptor::pull_sqls(const EventLines& event_lines, SQLs* sqls) {
    std::string in_ids;
    for (auto event_line : event_lines) {
        // format: action_type id
        if (event_line.size() != 2) {
            continue;
        }
        _pull_ids += event_line[1] + "|";
        in_ids += "'" + event_line[1] + "',";
    }
    in_ids[in_ids.length() - 1] = ')';
    std::string sql_str = _query_records_sql_prefix
            + s_bill_table_name + " where id in (" + in_ids;
    SQL sql = std::make_pair(SQLType::SELECT, sql_str);
    sqls->push_back(sql);
}

void BillMsgAdaptor::set_pull_records(const RecordLines& record_lines) {
    for (auto record_line : record_lines) {
        Record* record = _pull_bill_records->Add();
        for (auto it : record_line) {
            if (it.first == "id") {
                record->set_id(it.second);
            } else if (it.first == "year") {
                record->set_year(atoi(it.second.c_str()));
            } else if (it.first == "month") {
                record->set_month(atoi(it.second.c_str()));
            } else if (it.first == "day") {
                record->set_day(atoi(it.second.c_str()));
            } else if (it.first == "pay_earn") {
                record->set_pay_earn(atoi(it.second.c_str()));
            } else if (it.first == "gay") {
                record->set_gay(it.second);
            } else if (it.first == "comments") {
                record->set_comments(it.second);
            } else if (it.first == "cost") {
                record->set_cost(atoi(it.second.c_str()));
            } else if (it.first == "is_deleted") {
                record->set_is_deleted(atoi(it.second.c_str()));
            }
        }
    }
}

std::string BillMsgAdaptor::push_ids_str() {
    return "NEW:" + _push_new_ids + " UPDATE:" + _push_update_ids;
}

std::string BillMsgAdaptor::pull_ids_str() {
    return _pull_ids;
}

}

