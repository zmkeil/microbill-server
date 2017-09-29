
#ifndef APP_MICROBILL_EVENT_HANDLER_H
#define APP_MICROBILL_EVENT_HANDLER_H

#include <iostream>
#include <fstream>
#include <string>
#include "msg_adaptor.h"

namespace microbill {

struct Event {
	int index;
	EventLine event_line;
	Event(int index, EventLine& event_line) {
		this->index = index;
		this->event_line = event_line;
	}
};

class EventHandler {

public:
	EventHandler(std::string event_file) : _file(event_file) {}
	virtual ~EventHandler() {
        _fs.close();
    }

	bool init();
	bool get(int begin_index, int max_line, EventLines* event_lines);
	bool set(const EventLines& event_lines);
	
	int get_last_index() {
		return _last_index;
	}

private:
	std::string _file;
	std::fstream _fs;

	int _last_index;
	std::vector<Event> _events;
};

}

#endif //APP_MICROBILL_EVENT_HANDLER_H

