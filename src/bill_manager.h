#ifndef APP_MICROBILL_USER_MANAGER_H
#define APP_MICROBILL_USER_MANAGER_H

#include <hash_map>
#include <google/protobuf/repeated_field.h>
#include "bill_config.pb.h"
#include "bill.pb.h"
#include "pp_operator.h"
#include "billmsg_adaptor.h"

namespace microbill {

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

class BillManager {
public:
	BillManager();
	virtual ~BillManager();
	bool init(const BillOptions& bill_options, DBClient* db_client);

	bool push(const std::string& gay_name, BillMsgAdaptor* billmsg_adaptor);

	bool pull(const std::string& gay_name, int begin_index, int max_line, BillMsgAdaptor* billmsg_adaptor);

	int get_last_index(const std::string& gay_name);

private:
	PPOperator* _get_operator(const std::string& gay_name);

private:
	__gnu_cxx::hash_map<std::string, PPOperator*, StrHash, StrEqual> _users;
};

}

#endif
