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
	PPOperator(EventHandler* event_handler, DBHandler* db_handler) :
			_event_handler(event_handler), _db_handler(db_handler) {}
	virtual ~PPOperator() {}

    bool init();

	// for pull bills
	bool pull(int begin_index, int max_line, MsgAdaptor* msg_adaptor);

	// for push bills
	bool push(MsgAdaptor* msg_adaptor);

	int get_last_index() {
		return _event_handler->get_last_index();
	}

private:
	EventHandler* _event_handler;
	DBHandler* _db_handler;
};

}

#endif
