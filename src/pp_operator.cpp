
#include <comlog/info_log_context.h>
#include "pp_operator.h"

namespace microbill {

bool PPOperator::init() {
    _event_handler = new EventHandler(_event_file);
	if (!_event_handler->init()) {
		LOG(ERROR, "init PPOperator failed: _event_handler init error");
		return false;
	}

    _db_handler = new DBHandler(_db_client);
	if (!_db_handler->init()) {
		LOG(ERROR, "init PPOperator failed: _db_handler init error");
		return false;
	}
    return true;
}

bool PPOperator::pull(int begin_index, int max_line, MsgAdaptor* msg_adaptor) {
    if (!msg_adaptor) {
        return true;
    }
    EventLines event_lines;
    if (!_event_handler->get(begin_index, max_line, &event_lines)) {
        LOG(ERROR, "get events failed begin_index[%d] max_line[%d]", begin_index, max_line);
        return false;
    }
    if (event_lines.size() == 0) {
        // no more events
        return true;
    }

    SQLs sqls;
    msg_adaptor->pull_sqls(event_lines, &sqls);

    RecordLines record_lines;
    if (!_db_handler->pull_records(sqls, &record_lines)) {
        LOG(ERROR, "pull db records failed [%s]", (msg_adaptor->pull_ids_str()).c_str());
        return false;
    }
    msg_adaptor->set_pull_records(event_lines, record_lines);
    return true;
}


bool PPOperator::push(MsgAdaptor* msg_adaptor) {
    if (!msg_adaptor) {
        return true;
    }
    EventLines event_lines;
    SQLs sqls;
    msg_adaptor->push_sqls(&event_lines, &sqls);

    // first push db
    if (!_db_handler->push_records(sqls)) {
        LOG(ERROR, "push db records failed [%s]", (msg_adaptor->push_ids_str()).c_str());
        return false;
    }

    // second set events
    if (!_event_handler->set(event_lines)) {
        LOG(ERROR, "set events failed [%s]", (msg_adaptor->push_ids_str()).c_str());
        return false;
    }
    return true;
}

}
