
#ifndef APP_MICROBILL_BILLMSG_ADAPTOR_H
#define APP_MICROBILL_BILLMSG_ADAPTOR_H

#include <iostream>
#include <string>
#include <sstream>
#include "msg_adaptor.h"
#include "bill.pb.h"

namespace microbill {

class BillMsgAdaptor : public MsgAdaptor {

public:
	BillMsgAdaptor() {}
	virtual ~BillMsgAdaptor() {}

	// for push
	void push_sqls(EventLines*, SQLs*);

	// for pull
	void pull_sqls(const EventLines&, SQLs*);
	void set_pull_records(const EventLines&, const RecordLines&);

public:
    void set_push_bill_records(
            const ::google::protobuf::RepeatedPtrField<Record>* push_bill_records) {
        _push_bill_records = push_bill_records;
        // clear _push_ids
        _push_new_ids.clear();
        _push_update_ids.clear();
    }
    void set_pull_bill_records(::google::protobuf::RepeatedPtrField<Record>* pull_bill_records) {
        _pull_bill_records = pull_bill_records;
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
    static void set_bill_table_name(const std::string& table_name) {
        s_bill_table_name = table_name;
    }

private:
	const ::google::protobuf::RepeatedPtrField<Record>* _push_bill_records = NULL;
	::google::protobuf::RepeatedPtrField<Record>* _pull_bill_records = NULL;

    std::string _push_new_ids;
    std::string _push_update_ids;
    std::string _pull_ids;

private:
	const std::string _query_records_sql_prefix = "select id, year, month, day, pay_earn, gay, comments, cost, is_deleted from ";
	const std::string _insert_records_sql_snippet = " (id, year, month, day, pay_earn, gay, comments, cost, is_deleted) VALUES ";

private:
    static std::string s_bill_table_name;
};

}

#endif //APP_MICROBILL_MSG_ADAPTOR_H
