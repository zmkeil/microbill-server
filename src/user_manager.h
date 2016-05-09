#ifndef APP_MICROBILL_USER_MANAGER_H
#define APP_MICROBILL_USER_MANAGER_H

#include <hash_map>
#include <google/protobuf/repeated_field.h>
#include "bill_config.pb.h"
#include "pending_events.h"
#include "bill_context.h"

namespace microbill {

struct UserInfo {
	std::string name;
	std::string event_file;
	PendingEvents* pending_events;
	UserInfo(std::string name, std::string event_file) :
		name(name), event_file(event_file),
		pending_events(NULL) {}
};

struct StrHash {
	size_t operator()(const std::string& str) const {
		return __gnu_cxx::__stl_hash_string(str.c_str());
	}
};

struct StrEqual {
	size_t operator()(const std::string& l, const std::string& r) {
		return l == r;
	}
};

class UserManager {
public:
	UserManager();
	virtual ~UserManager();
	bool init(const ::google::protobuf::RepeatedPtrField<UserOptions>& user_options);

	bool set_events_for_others(std::string self_name,
			const ::google::protobuf::RepeatedPtrField<Record>& new_records, BillContext* context);
	bool get_events_for_self(std::string self_name, int begin_index, int max_line,
			DBHelper* db_helper, ::google::protobuf::RepeatedPtrField<Record>* updated_records, BillContext* context);

private:
	UserInfo* get_user(std::string name);

private:
	__gnu_cxx::hash_map<std::string, UserInfo*, StrHash, StrEqual> _users;
};

}

#endif
