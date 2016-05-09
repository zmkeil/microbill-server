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
		::google::protobuf::RepeatedPtrField<Record>* updated_records, BillContext* context)
{
	(void) context;
	std::vector<RecordContent> record_contents;
	if (!_client->query_records(ids, &record_contents)) {
		LOG(ERROR, "query_records failed");
		return false;
	}

	if (ids.size() != record_contents.size()) {
		// warning
		LOG(NOTICE, "the size of records(%d) and query_contents(%d) not the same, maybe some records don't have content in DB", ids.size(), record_contents.size());
	}
	
	// fill updated_records with record_content
	std::map<std::string, RecordContent*> content_map;
	// NOTE! we cann't guess the order of results returned by DB
	// build a simple map for index
	int content_size = (int)record_contents.size();
	for (int i = 0; i < content_size; ++i) {
		content_map[record_contents[i].id] = &record_contents[i];
	}
	int record_size = updated_records->size();
	for (int i = 0; i < record_size; ++i) {
		Record* record = updated_records->Mutable(i);
		auto it = content_map.find(record->id());
		if (it == content_map.end()) {
			LOG(NOTICE, "the %dth record (id=\"%s\") does not have content in DB",
					i, record->id().c_str());
			continue;
		}

		const RecordContent* content = it->second;
		record->set_year(content->year);
		record->set_month(content->month);
		record->set_day(content->day);
		record->set_pay_earn(content->pay_earn);
		record->set_gay(content->gay);
		record->set_comments(content->comments);
		record->set_cost(content->cost);
		record->set_is_deleted(content->is_deleted);
		//it++;
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
static std::string track_modify_records(const Record& record, std::vector<ModifyRecordPair>* modify_records)
{
	std::string str;
	if (!record.has_id()) {
		LOG(WARN, "modified record don't have id");
		return str;
	}
	str += record.id();
	str += ",";
//	if (record.has_year() || record.has_month()) {
//		LOG(WARN, "can't modify year or month");
//		return str;
//	}
	if (record.has_day()) {
		std::stringstream ss;
		ss << record.day();
		modify_records->emplace_back("day", ss.str());
		str += ss.str();
		str += ",";
	}
	if (record.has_pay_earn()) {
		modify_records->emplace_back("pay_earn", record.pay_earn() ? "1" : "0");
	}
	if (record.has_gay()) {
		std::string gay = "'" + record.gay() + "'";
		modify_records->emplace_back("gay", gay);
		str += gay;
		str += ",";
	}
	if (record.has_comments()) {
		std::string comments = "'" + record.comments() + "'";
		modify_records->emplace_back("comments", comments);
		str += comments;
		str += ",";
	}
	if (record.has_cost()) {
		std::stringstream ss;
		ss << record.cost();
		modify_records->emplace_back("cost", ss.str());
		str += ss.str();
		str += ",";
	}
	if (record.has_is_deleted()) {
		std::string is_deleted = record.is_deleted() ? "1" : "0";
		modify_records->emplace_back("is_deleted", is_deleted);
		str += is_deleted;
		str += ",";
	}

	if (modify_records->size() == 0) {
		return str;
	} 
	if ((modify_records->back().first) == std::string("id")) {
		LOG(WARN, "this modify record don't have changes");
		return str;
	}
	std::string id = "'" + record.id() + "'";
	modify_records->emplace_back("id", id);
	return str;
}

static std::string record_to_string(const Record& record)
{
	char buffer[1024];
	snprintf(buffer, 1024, "%s,%d,%s,%s,%d", record.id().c_str(),
			record.day(), record.gay().c_str(), record.comments().c_str(), record.cost());
	std::string str(buffer);
	return str;
}

bool DBHelper::push_records(const ::google::protobuf::RepeatedPtrField<Record>& new_records, BillContext* context)
{
	std::vector<RecordContent> new_record_contents;
	std::vector<ModifyRecordPair> modify_records;
	std::string all_new_records;
	std::string all_update_records;

	int record_size = new_records.size();
	for (int i = 0; i < record_size; ++i) {
		const Record& record = new_records.Get(i);
		if (record.type() == Record::NEW) {
			new_record_contents.emplace_back(record.id(), record.year(), record.month(),
					record.day(), record.pay_earn(), record.gay(), record.comments(),
					record.cost(), record.is_deleted());
			all_new_records += record_to_string(record);
			all_new_records += "|";

		} else {
			all_update_records += track_modify_records(record, &modify_records);
			all_update_records += "|";
		}
	}
	if (context) {
		context->set_session_field(std::string("new_records"), all_new_records);
		context->set_session_field(std::string("update_records"), all_update_records);
	}

	return _client->push_records(new_record_contents, modify_records);
}

}
