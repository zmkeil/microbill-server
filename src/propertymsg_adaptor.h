
#ifndef APP_MICROBILL_PROPERTYMSG_ADAPTOR_H
#define APP_MICROBILL_PROPERTYMSG_ADAPTOR_H

#include <iostream>
#include <string>
#include <sstream>
#include "msg_adaptor.h"
#include "bill.pb.h"

namespace microbill {

typedef PropertyRecord_PocketRecord PocketRecord;
typedef PropertyRecord_AssetsRecord AssetsRecord;

class PropertyMsgAdaptor : public MsgAdaptor {

public:
	PropertyMsgAdaptor() {}
	virtual ~PropertyMsgAdaptor() {}

	// for push
	void push_sqls(EventLines*, SQLs*);

	// for pull
	void pull_sqls(const EventLines&, SQLs*);
	void set_pull_records(const EventLines&, const RecordLines&);

public:
    void set_push_property_records(
            const ::google::protobuf::RepeatedPtrField<PropertyRecord>* push_property_records) {
        _push_property_records = push_property_records;
        // clear _push_ids
        _push_new_ids.clear();
        _push_update_ids.clear();
    }
    void set_pull_property_records(::google::protobuf::RepeatedPtrField<PropertyRecord>* pull_property_records) {
        _pull_property_records = pull_property_records;
        // clear _pull_ids
        _pull_ids.clear();
    }
    std::string push_ids_str() {
        return "NEW:" + _push_new_ids + " UPDATE:" + _push_update_ids;
    }
    std::string pull_ids_str() {
        return _pull_ids;
    }

public:
    static void set_pocket_table_name(const std::string& table_name) {
        s_pocket_table_name = table_name;
    }
    static void set_assets_table_name(const std::string& table_name) {
        s_assets_table_name = table_name;
    }

private:
	const ::google::protobuf::RepeatedPtrField<PropertyRecord>* _push_property_records = NULL;
	::google::protobuf::RepeatedPtrField<PropertyRecord>* _pull_property_records = NULL;

    std::string _push_new_ids;
    std::string _push_update_ids;
    std::string _pull_ids;

private:
	const std::string _query_pocket_sql_prefix = "select sid, year, month, comments, money, is_deleted from ";
	const std::string _query_assets_sql_prefix = "select sid, year, month, day, store_addr, flow_type, money, store_addr_op, is_deleted from ";
	const std::string _insert_pocket_sql_snippet = " (sid, year, month, comments, money, is_deleted) VALUES ";
	const std::string _insert_assets_sql_snippet = " (sid, year, month, day, store_addr, flow_type, money, store_addr_op, is_deleted) VALUES ";

private:
    static std::string s_pocket_table_name;
    static std::string s_assets_table_name;
};

}

#endif //APP_MICROBILL_MSG_ADAPTOR_H
