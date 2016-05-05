#ifndef APP_MICROBILL_BILL_CONTEXT_H
#define APP_MICROBILL_BILL_CONTEXT_H

#include <map>
#include <algorithm>
#include <service_context.h>
//#include "db_helper.h"
//#include "user_manager.h"

namespace microbill {

class DBHelper;
class UserManager;

struct BillContext : public nrpc::ServiceContext
{
	DBHelper* db_helper;
	UserManager* user_manager;

	BillContext(DBHelper* db_helper, UserManager* user_manager) :
		db_helper(db_helper), user_manager(user_manager), _delimiter("^,") {}

	void build_log(std::string* log) {
		std::for_each(_session_context.begin(), _session_context.end(),
			[&log, this] (std::pair<std::string, std::string> p) {
				(*log) += p.first;
				(*log) += ":";
				(*log) += p.second;
				(*log) += _delimiter;
			});
		return;
	}

	void set_session_field(std::string key, std::string value) {
		_session_context.insert(std::pair<std::string, std::string>(key, value));
	}

private:
	std::map<std::string, std::string> _session_context;
	std::string _delimiter;
};

class BillContextFactory : public nrpc::ServiceContextFactory
{
public:
	BillContextFactory(DBHelper* db_helper, UserManager* user_manager) :
		_db_helper(db_helper), _user_manager(user_manager) {}
	virtual ~BillContextFactory() {}

	nrpc::ServiceContext* create_context() {
		return new BillContext(_db_helper, _user_manager);
	}

private:
	DBHelper* _db_helper;
	UserManager* _user_manager;
};

}

#endif

