#ifndef APP_MICROBILL_PENDING_EVENTS_H
#define APP_MICROBILL_PENDING_EVENTS_H

#include <iostream>
#include <fstream>
#include <string>
#include <google/protobuf/repeated_field.h>
#include "bill.pb.h"
#include "db_helper.h"

namespace microbill {

struct Event {
	int index;
	Record_Type action;
	// bill.id in mysql
	std::string id;
	
	Event(int index, Record_Type action, std::string id) {
		this->index = index;
		this->action = action;
		this->id = id;
	}
};

class PendingEvents {

public:
	PendingEvents(std::string event_file) : _file(event_file) {}
	virtual ~PendingEvents() {}

	bool init();
	//bool reload();

	bool get(int begin_index, int max_line, DBHelper* db_helper,
			::google::protobuf::RepeatedPtrField<Record>* updated_records, BillContext* context);

	bool set(const ::google::protobuf::RepeatedPtrField<Record>& new_records);
			
	int get_last_index();

private:
	std::string _file;
	std::fstream _fs;

	int _first_index;
	int _last_index;
	std::vector<Event> _events;
};

}

#endif
