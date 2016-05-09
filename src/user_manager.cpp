
#include <iostream>
#include <string>
#include <comlog/info_log_context.h>
#include "user_manager.h"

namespace microbill {

UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

bool UserManager::init(const ::google::protobuf::RepeatedPtrField<UserOptions>& user_options)
{
	int o_size = user_options.size();
	for (int i = 0; i < o_size; ++i) {
		const UserOptions& user = user_options.Get(i);
		UserInfo* uinfo = new UserInfo(user.name(), user.file_name());
		uinfo->pending_events = new PendingEvents(user.file_name());
		if (!uinfo->pending_events->init()) {
			LOG(ERROR, "Failed to init user \"%s\"", user.name().c_str());
			return false;
		}
		_users[user.name()] = uinfo;
	}
	return true;
}

bool UserManager::set_events_for_others(std::string self_name,
		const ::google::protobuf::RepeatedPtrField<Record>& new_records, BillContext* context)
{
	(void) context;
	bool ret = true;
	// self_name must be registered
	if (!get_user(self_name)) {
		LOG(ERROR, "this user not register");
		return false;
	}
	for (auto it = _users.begin(); it != _users.end(); ++it) {
		if (self_name == it->first) {
			continue;
		}
		PendingEvents* events = (it->second)->pending_events;
		if (!(events->set(new_records))) {
			ret = false;
		}
	}
	return ret;
}

bool UserManager::get_events_for_self(std::string self_name, int begin_index, int max_line,
		DBHelper* db_helper, ::google::protobuf::RepeatedPtrField<Record>* updated_records, BillContext* context)
{
	UserInfo* self = get_user(self_name);
	if (!self) {
		return false;
	}
	PendingEvents* events = self->pending_events;
	if (!events) {
		return false;
	}

	if (context) {
		char buffer[256];
		snprintf(buffer, 256, "[%d,%d]", begin_index, max_line);
		std::string other_event(buffer);
		context->set_session_field("sync_index", other_event);
	}
	return events->get(begin_index, max_line, db_helper, updated_records, context);
}

UserInfo* UserManager::get_user(std::string name)
{
	auto it = _users.find(name);
	if (it == _users.end()) {
		return NULL;
	}
	return it->second;
}

}
