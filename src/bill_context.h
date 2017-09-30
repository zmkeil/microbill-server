#ifndef APP_MICROBILL_BILL_CONTEXT_H
#define APP_MICROBILL_BILL_CONTEXT_H

#include <map>
#include <algorithm>
#include <service_context.h>

namespace microbill {

class BillManager;

struct LogContextPair {
	std::string key;
	std::string value;
	LogContextPair(std::string _key, std::string _value) :
		key(_key), value(_value) {}
};

struct BillContext : public nrpc::ServiceContext
{
	BillManager* bill_manager;

	BillContext(BillManager* bill_manager) :
		bill_manager(bill_manager), _delimiter("^,") {}

	void build_log(std::string* log) {
		std::for_each(_session_context.begin(), _session_context.end(),
			[&log, this] (LogContextPair p) {
				(*log) += p.key;
				(*log) += ":";
				(*log) += p.value;
				(*log) += _delimiter;
			});
		return;
	}

	void set_session_field(std::string key, std::string value) {
		_session_context.emplace_back(key, value);
	}

private:
	std::vector<LogContextPair> _session_context;
	std::string _delimiter;
};

class BillContextFactory : public nrpc::ServiceContextFactory
{
public:
	BillContextFactory(BillManager* bill_manager) :
		_bill_manager(bill_manager) {}
	virtual ~BillContextFactory() {}

	nrpc::ServiceContext* create_context() {
		return new BillContext(_bill_manager);
	}

private:
	BillManager* _bill_manager;
};

}

#endif

