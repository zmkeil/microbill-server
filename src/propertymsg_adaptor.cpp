
#include <iostream>
#include <string>
#include <comlog/info_log_context.h>
#include "propertymsg_adaptor.h"

namespace microbill {

// event format: action_type property_type sid

std::string PropertyMsgAdaptor::s_pocket_table_name = "unkown";
std::string PropertyMsgAdaptor::s_assets_table_name = "unkown";

PropertyRecord_AssetsRecord::StoreAddr s_store_addr_value[8] = {
    PropertyRecord_AssetsRecord_StoreAddr_ZM_EBAO,
    PropertyRecord_AssetsRecord_StoreAddr_ZM_FUND,
    PropertyRecord_AssetsRecord_StoreAddr_ZM_TBJ,
    PropertyRecord_AssetsRecord_StoreAddr_ZM_WD,
    PropertyRecord_AssetsRecord_StoreAddr_ZM_SZYH,
    PropertyRecord_AssetsRecord_StoreAddr_JXJ_TBJ,
    PropertyRecord_AssetsRecord_StoreAddr_JXJ_LJS,
    PropertyRecord_AssetsRecord_StoreAddr_JXJ_SZYH
};

PropertyRecord_AssetsRecord::FlowType s_flow_type_value[5] = {
    PropertyRecord_AssetsRecord_FlowType_STORE,
    PropertyRecord_AssetsRecord_FlowType_INTEREST,
    PropertyRecord_AssetsRecord_FlowType_DRAW,
    PropertyRecord_AssetsRecord_FlowType_DEBT,
    PropertyRecord_AssetsRecord_FlowType_FLOW
};

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
        event_line.push_back(property_type == PropertyRecord::POCKET_MONEY ? "p" : "a");
        event_line.push_back(sid);
        event_lines->push_back(event_line);
    }
}

void PropertyMsgAdaptor::pull_sqls(const EventLines& event_lines, SQLs* sqls) {
    std::string in_pocket_ids;
    std::string in_assets_ids;
    for (auto event_line : event_lines) {
        // format: action_type property_type sid
        if (event_line.size() != 3) {
            continue;
        }
        if (event_line[1] == "p") {
            _pull_ids += "p_" + event_line[2] + "|";
            in_pocket_ids += "'" + event_line[2] + "',";
        } else if (event_line[1] == "a") {
            _pull_ids += "a_" + event_line[2] + "|";
            in_assets_ids += "'" + event_line[2] + "',";
        }
    }
    if (in_pocket_ids.size() > 0) {
        in_pocket_ids[in_pocket_ids.length() - 1] = ')';
        std::string sql_str = _query_pocket_sql_prefix
                + s_pocket_table_name + " where sid in (" + in_pocket_ids;
        SQL sql = std::make_pair(SQLType::SELECT, sql_str);
        sqls->push_back(sql);
    }
    if (in_assets_ids.size() > 0) {
        in_assets_ids[in_assets_ids.length() - 1] = ')';
        std::string sql_str = _query_assets_sql_prefix
                + s_assets_table_name + " where sid in (" + in_assets_ids;
        SQL sql = std::make_pair(SQLType::SELECT, sql_str);
        sqls->push_back(sql);
    }
}

void PropertyMsgAdaptor::set_pull_records(const EventLines& event_lines,
        const RecordLines& record_lines) {
    if (!_pull_property_records) {
        return;
    }
    std::map<std::string/*type_sid*/, PropertyRecord_Type/*action*/> action_map;
    for (auto event_line : event_lines) {
        std::string key = event_line[1] + "_" + event_line[2];
        if (action_map.find(key) != action_map.end()) {
            // already exist, only can be NEW - UPDATE - UPDATE ...
            // so just use NEW
            continue;
        }
        action_map[key] = (event_line[0] == "0" ? PropertyRecord::NEW : PropertyRecord::UPDATE);
    }
    for (auto record_line : record_lines) {
        // sid
        if (record_line.find("sid") == record_line.end()) {
            continue;
        }
        std::string sid = record_line["sid"];
        // property_type
        PropertyRecord_PropertyType property_type = (record_line.find("store_addr") != record_line.end() ?
                PropertyRecord::FIXED_ASSETS : PropertyRecord::POCKET_MONEY);
        // key ==> action
        std::string key = (property_type == PropertyRecord::FIXED_ASSETS ? "a_" : "p_") + sid;
        PropertyRecord_Type action_type =
                (action_map.find(key) != action_map.end()) ? action_map[key] : PropertyRecord::NEW;

        PropertyRecord* record = _pull_property_records->Add();
        record->set_type(action_type);
        record->set_property_type(property_type);

        if (property_type == PropertyRecord::POCKET_MONEY) {
            if (record_line["year"] == "" ||
                    record_line["month"] == "" ||
                    //record_line["comments"] == "" ||
                    record_line["money"] == "" ||
                    record_line["is_deleted"] == "") {
                LOG(WARN, "bad pocket record sid = %s", sid.c_str());
                continue;
            }
            PropertyRecord_PocketRecord* pocket_record = record->mutable_pocket_record();
            pocket_record->set_sid(sid);
            pocket_record->set_year(atoi(record_line["year"].c_str()));
            pocket_record->set_month(atoi(record_line["month"].c_str()));
            //pocket_record->set_comments(record_line["comments"]);
            pocket_record->set_money(atoi(record_line["money"].c_str()));
            pocket_record->set_is_deleted(atoi(record_line["is_deleted"].c_str()));
            LOG(WARN, "good pocket record sid = %s", sid.c_str());
        }
        else if (property_type == PropertyRecord::FIXED_ASSETS) {
            if (record_line["year"] == "" ||
                    record_line["month"] == "" ||
                    record_line["day"] == "" ||
                    record_line["store_addr"] == "" ||
                    record_line["flow_type"] == "" ||
                    record_line["money"] == "" ||
                    record_line["store_addr_op"] == "" ||
                    record_line["is_deleted"] == "") {
                LOG(WARN, "bad assets record sid = %s", sid.c_str());
                continue;
            }
            PropertyRecord_AssetsRecord* assets_record = record->mutable_assets_record();
            assets_record->set_sid(sid);
            assets_record->set_year(atoi(record_line["year"].c_str()));
            assets_record->set_month(atoi(record_line["month"].c_str()));
            assets_record->set_day(atoi(record_line["day"].c_str()));
            assets_record->set_store_addr(s_store_addr_value[atoi(record_line["store_addr"].c_str())]);
            assets_record->set_flow_type(s_flow_type_value[atoi(record_line["flow_type"].c_str())]);
            assets_record->set_money(atoi(record_line["money"].c_str()));
            assets_record->set_store_addr_op(s_store_addr_value[atoi(record_line["store_addr_op"].c_str())]);
            assets_record->set_is_deleted(atoi(record_line["is_deleted"].c_str()));
        }
    }
}

}

