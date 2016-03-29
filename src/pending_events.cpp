#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <comlog/info_log_context.h>
#include "pending_events.h"
#include "db_helper.h"

namespace microbill {

struct EventComp
{
	bool operator()(Event event_it, Event event) {
		return event_it.index < event.index;
	}

	bool operator()(Event event_it, int index) {
		return event_it.index < index;
	}
};

static bool get_event(char* buf, std::vector<Event>& events)
{
	char *p = strtok(buf, " ");
	if ((!p) || (p[0] == '#')) return true;
	int index = atoi(p);
	p = strtok(NULL, " ");
	if (!p) return false;
	Record_Type action = (Record_Type)atoi(p);
	p = strtok(NULL, " ");
	if (!p) return false;
	std::string id(p);

	events.emplace_back(index, action, id);
	return true;
}

bool PendingEvents::init()
{
	_fs.open(_file, std::ios::in|std::ios::out|std::ios::app);
	if (!_fs.good()) {
		return false;
	}
	char buf[1024];
	while (true) {
		_fs.getline(buf, 1024);
		if (_fs.eof()) {
			break;
		}
		if (_fs.fail()) {
			return false;
		}
		// _fs.good()
		get_event(buf, _events);
	}
	if (_events.size() > 0) {
		_last_index = _events[_events.size() - 1].index;
	} else {
		_last_index = 0;
	}
	_fs.close();
	_fs.open(_file, std::ios::out|std::ios::app);
	return true;
}

bool PendingEvents::get(int begin_index, int max_line, DBHelper* db_helper,
		::google::protobuf::RepeatedPtrField<Record>* updated_records)
{
	auto it = std::lower_bound(_events.begin(), _events.end(), begin_index, EventComp());
	if (it == _events.end() || (it)->index != begin_index) {
		// begin_index is too bigger, or not existed in updated_records
		return false;
	}

	std::vector<std::string> ids;
	for (;it != _events.end(); ++it) {
		Record* record = updated_records->Add();
		record->set_type((it)->action);
		record->set_id((it)->id);
		
		ids.push_back((it)->id);
		if (--max_line == 0) {
			break;
		}
	}

	// get content from DB
	return db_helper->get_records_by_id_list(ids, updated_records);
}

bool PendingEvents::set(const ::google::protobuf::RepeatedPtrField<Record>& new_records)
{
	for (int i = 0; i < new_records.size(); ++i) {
		const Record& record = new_records.Get(i);
		_last_index++;
		_fs << _last_index << " ";
		_fs << record.type() << " ";
		_fs << record.id() << "\n";
		if (!_fs.good()) {
			LOG(ERROR, "write events files failed");
			return false;
		}

		_events.emplace_back(_last_index, record.type(), record.id());
	}
	_fs.flush();
	if (!_fs.good()) {
		LOG(ERROR, "flush events files failed");
		return false;
	}
	return true;
}

}
