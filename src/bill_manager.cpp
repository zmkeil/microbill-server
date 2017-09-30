
#include <iostream>
#include <string>
#include <comlog/info_log_context.h>
#include "bill_manager.h"
#include "db_handler.h"
#include "event_handler.h"
#include "billmsg_adaptor.h"

namespace microbill {

BillManager::BillManager()
{
}

BillManager::~BillManager()
{
    for (auto it : _users) {
        if (it.second) {
            delete it.second;
        }
    }
}

bool BillManager::init(const BillOptions& bill_options, DBClient* db_client)
{
	int o_size = bill_options.users().size();
	for (int i = 0; i < o_size; ++i) {
		const UserOptions& user = bill_options.users().Get(i);
        std::string user_name = user.name();
        std::string event_file_name = user.file_name();
        PPOperator* pp_operator = new PPOperator(event_file_name, db_client);
        if (!pp_operator->init()) {
            LOG(ERROR, "Failed to init \"%s\"'s pp_operator", user_name.c_str());
            return false;
        }
		_users[user_name] = pp_operator;
	}

    BillMsgAdaptor::set_bill_table_name(bill_options.table_name());
	return true;
}

bool BillManager::push(const std::string& gay_name, BillMsgAdaptor* billmsg_adaptor)
{
	// gay_name must be registered
	PPOperator* pp_operator = _get_operator(gay_name);
	if (!pp_operator) {
		LOG(ERROR, "push bill failed: \"%s\" not register", gay_name.c_str());
		return false;
	}

	if (!pp_operator->push(billmsg_adaptor)) {
		LOG(ERROR, "push bill failed: pp_operate error", gay_name.c_str());
		return false;
	}
	return true;
}

bool BillManager::pull(const std::string& gay_name, int begin_index, int max_line, BillMsgAdaptor* billmsg_adaptor)
{
	PPOperator* pp_operator = _get_operator(gay_name);
	if (!pp_operator) {
        LOG(ERROR, "pull bill failed: \"%s\" not register", gay_name.c_str());
		return false;
	}

	if (!pp_operator->pull(begin_index, max_line, billmsg_adaptor)) {
        LOG(ERROR, "pull bill failed: pp_operate error", gay_name.c_str());
        return false;
	}
	return true;
}

int BillManager::get_last_index(const std::string& gay_name) {
	PPOperator* pp_operator = _get_operator(gay_name);
	if (!pp_operator) {
		return 0;
	}
	return pp_operator->get_last_index();
}

PPOperator* BillManager::_get_operator(const std::string& gay_name)
{
	auto it = _users.find(gay_name);
	if (it == _users.end()) {
		return NULL;
	}
	return it->second;
}

}
