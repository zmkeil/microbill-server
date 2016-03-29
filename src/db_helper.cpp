#include <iostream>
#include <sstream>
#include <comlog/info_log_context.h>
#include "db_helper.h"

namespace microbill {

/* 
 * ids.size() equal updated_records.size()
 * this function fill the updated_records with query result from DB,
 * but it doesn't gurant that evety update_record will be filled because
 * some id may not exist in DB
 */
bool DBHelper::get_records_by_id_list(const std::vector<std::string>& ids,
		::google::protobuf::RepeatedPtrField<Record>* updated_records)
{
	std::vector<RecordContent> record_contents;
	if (!_client->query_records(ids, &record_contents)) {
		LOG(ERROR, "query_records failed");
		return false;
	}

	if (ids.size() != record_contents.size()) {
		// warning
		LOG(NOTICE, "the size of records(%d) and query_contents(%d) not the same, maybe some records don't have content in DB", record_contents.size(), ids.size());
	}
	
	// fill updated_records with record_content
	std::vector<RecordContent>::iterator it = record_contents.begin();
	int o_size = ids.size();
	for (int i = 0; i < o_size; ++i) {
		Record* record = updated_records->Mutable(i);
		if ((it == record_contents.end()) || (record->id().size() != it->id.size()) ||
				memcmp(record->id().c_str(), it->id.c_str(), record->id().size())) {
			LOG(NOTICE, "the %dth record (id=\"%s\":\"%s\") does not have content in DB", i,
					record->id().c_str(), (it == record_contents.end()) ? "NULL" : it->id.c_str());
			continue;
		}

		const RecordContent& content = *it;
		record->set_year(content.year);
		record->set_month(content.month);
		record->set_day(content.day);
		record->set_pay_earn(content.pay_earn);
		record->set_gay(content.gay);
		record->set_comments(content.comments);
		record->set_cost(content.cost);
		record->set_is_deleted(content.is_deleted);
		it++;
	}
	return true;
}


/* 
 * the modify_records contain severy records, for example:
 *
 * day : 18
 * comments : lunch
 * id : zmkeil_2016_03_35	// mark one record
 * cost : 34
 * id : jxj_2016_03_32 // next record
 */
static void push_modify_records(const Record& record, std::vector<ModifyRecordPair>* modify_records)
{
	if (!record.has_id()) {
		LOG(NOTICE, "modified record don't have id");
		return;
	}
	if (record.has_year() || record.has_month()) {
		LOG(NOTICE, "can't modify year or month");
		return;
	}
	if (record.has_day()) {
		std::stringstream ss;
		ss << record.day();
		modify_records->emplace_back("day", ss.str());
	}
	if (record.has_pay_earn()) {
		modify_records->emplace_back("pay_earn", record.pay_earn() ? "1" : "0");
	}
	if (record.has_gay()) {
		std::string gay = "'" + record.gay() + "'";
		modify_records->emplace_back("gay", gay);
	}
	if (record.has_comments()) {
		std::string comments = "'" + record.comments() + "'";
		modify_records->emplace_back("comments", comments);
	}
	if (record.has_cost()) {
		std::stringstream ss;
		ss << record.cost();
		modify_records->emplace_back("cost", ss.str());
	}
	if (record.has_is_deleted()) {
		modify_records->emplace_back("is_deleted", record.is_deleted() ? "1" : "0");
	}

	if (modify_records->size() == 0) {
		return;
	} 
	if ((modify_records->end()--)->first == std::string("id")) {
		LOG(NOTICE, "this modify record don't have changes");
		return;
	}
	std::string id = "'" + record.id() + "'";
	modify_records->emplace_back("id", id);
}

bool DBHelper::push_records(const ::google::protobuf::RepeatedPtrField<Record>& new_records)
{
	std::vector<RecordContent> new_record_contents;
	std::vector<ModifyRecordPair> modify_records;

	int o_size = new_records.size();
	for (int i = 0; i < o_size; ++i) {
		const Record& record = new_records.Get(i);
		if (record.type() == Record::NEW) {
			new_record_contents.emplace_back(record.id(), record.year(), record.month(),
					record.day(), record.pay_earn(), record.gay(), record.comments(),
					record.cost(), record.is_deleted());
		} else {
			push_modify_records(record, &modify_records);
		}
	}

	return _client->push_records(new_record_contents, modify_records);
}

}
