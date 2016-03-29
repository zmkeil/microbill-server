#ifndef APP_MICROBILL_DB_HELPER_H
#define APP_MICROBILL_DB_HELPER_H

#include <vector>
#include <google/protobuf/repeated_field.h>
#include "bill.pb.h"

namespace microbill {

struct RecordContent {
	std::string id;
	int year;
	int month;
	int day;
	int pay_earn;
	std::string gay;
	std::string comments;
	int cost;
	int is_deleted;

	RecordContent(std::string id, int year, int month, int day, int pay_earn, std::string gay,
			std::string comments, int cost, int is_deleted) :
		id(id), year(year), month(month), day(day), pay_earn(pay_earn),
		gay(gay), comments(comments), cost(cost), is_deleted(is_deleted) {}

	RecordContent() : id("NULL") {}

};

typedef std::pair<std::string, std::string> ModifyRecordPair;

class DBClient {
public:
	DBClient() {}
	virtual ~DBClient() {}

	virtual bool query_records(const std::vector<std::string>& ids, std::vector<RecordContent>* record_contents) = 0;

	virtual bool push_records(const std::vector<RecordContent>& new_record_contents, const std::vector<ModifyRecordPair>& modify_records) = 0;

};


class DBHelper {
public:
	DBHelper(DBClient* client) : _client(client) {}
	virtual ~DBHelper() {}

	bool get_records_by_id_list(const std::vector<std::string>& ids,
			::google::protobuf::RepeatedPtrField<Record>* updated_records);

	bool push_records(const ::google::protobuf::RepeatedPtrField<Record>& new_records);

private:
	DBClient* _client;
};

}

#endif
