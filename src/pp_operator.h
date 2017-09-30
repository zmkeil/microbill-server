#ifndef APP_MICROBILL_PP_OPERATOR_H
#define APP_MICROBILL_PP_OPERATOR_H

#include <iostream>
#include <string>
#include "msg_adaptor.h"
#include "event_handler.h"
#include "db_handler.h"

namespace microbill {

class PPOperator {

public:
	PPOperator(const std::string& event_file, DBClient* db_client) :
			_event_file(event_file), _db_client(db_client) {}
	virtual ~PPOperator() {
        if (_event_handler) {
            delete _event_handler;
        }
        if (_db_handler) {
            delete _db_handler;
        }
    }

    bool init();

	// for pull bills
	bool pull(int begin_index, int max_line, MsgAdaptor* msg_adaptor);

	// for push bills
	bool push(MsgAdaptor* msg_adaptor);

	int get_last_index() {
		return _event_handler->get_last_index();
	}

private:
    std::string _event_file;
    DBClient* _db_client;

private:
	EventHandler* _event_handler = NULL;
	DBHandler* _db_handler = NULL;
};

}

#endif
