#ifndef APP_MICROBILL_USER_MANAGER_H
#define APP_MICROBILL_USER_MANAGER_H

#include <map>
#include <google/protobuf/repeated_field.h>
#include "bill_config.pb.h"
#include "bill.pb.h"
#include "pp_operator.h"
#include "propertymsg_adaptor.h"

namespace microbill {

// gay_name only used to check push/pull privileges

class PropertyManager {
public:
	PropertyManager();
	virtual ~PropertyManager();
	bool init(const PropertyOptions& bill_options, DBClient* db_client);

	bool push(const std::string& gay_name, PropertyMsgAdaptor* propertymsg_adaptor);

	bool pull(const std::string& gay_name, int begin_index, int max_line, PropertyMsgAdaptor* propertymsg_adaptor);

	int get_last_index();

private:
	PPOperator* _pp_operator;
};

}

#endif
