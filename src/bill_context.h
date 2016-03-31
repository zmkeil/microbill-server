#ifndef APP_MICROBILL_BILL_CONTEXT_H
#define APP_MICROBILL_BILL_CONTEXT_H

#include "service_context.h"

namespace microbill {

struct BillContext : public nrpc::ServiceContext
{
	DBHelper* db_helper;
	UserManager* user_manager;

	BillContext(DBHelper* db_helper, UserManager* user_manager) :
		db_helper(db_helper), user_manager(user_manager) {}

	void build_log(std::string* log) {
		(void) log;
		return;
	}
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

