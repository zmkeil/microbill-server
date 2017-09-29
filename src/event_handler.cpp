#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <comlog/info_log_context.h>
#include "event_handler.h"

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

static bool get_event(char* buf, std::vector<Event>* events)
{
	char *p = strtok(buf, " ");
	if ((!p) || (p[0] == '#')) return true;
	int index = atoi(p);
    EventLine event_line;
    while (1) {
        p = strtok(NULL, " ");
        if (!p) break;
        event_line.push_back(p);
    }

	events->emplace_back(index, event_line);
	return true;
}

bool EventHandler::init()
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
		get_event(buf, &_events);
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

bool EventHandler::get(int begin_index, int max_line, EventLines* event_lines)
{
	auto it = std::lower_bound(_events.begin(), _events.end(), begin_index, EventComp());
	if (it == _events.end() || (it)->index != begin_index) {
		// begin_index is too bigger, or not existed in updated_records
		return false;
	}

	std::vector<std::string> ids;
	std::string ids_str;
	for (; it != _events.end(); ++it) {
        event_lines->push_back(it->event_line);
		if (--max_line == 0) {
			break;
		}
	}
	return true;
}

bool EventHandler::set(const EventLines& event_lines)
{
	for (EventLine event_line : event_lines) {
		_last_index++;
		_fs << _last_index;
        for (std::string item : event_line) {
            _fs << " " << item;
        }
		_fs << "\n";
		if (!_fs.good()) {
			LOG(ERROR, "write events files failed index [%d]", _last_index);
			return false;
		}
		_events.emplace_back(_last_index, event_line);
	}

	_fs.flush();
	if (!_fs.good()) {
		LOG(ERROR, "flush events files failed");
		return false;
	}
	return true;
}

}
