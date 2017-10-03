
#include <iostream>
#include <string>
#include <comlog/info_log_context.h>
#include "property_manager.h"
#include "db_handler.h"
#include "event_handler.h"
#include "propertymsg_adaptor.h"

namespace microbill {

PropertyManager::PropertyManager()
{
}

PropertyManager::~PropertyManager()
{
    if (_pp_operator) {
        delete _pp_operator;
    }
}

bool PropertyManager::init(const PropertyOptions& property_options, DBClient* db_client)
{
    std::string event_file_name = property_options.file_name();
    _pp_operator = new PPOperator(event_file_name, db_client);
    if (!_pp_operator->init()) {
        LOG(ERROR, "Failed to init property_manager's pp_operator");
        return false;
    }

    PropertyMsgAdaptor::set_pocket_table_name(property_options.pocket_table_name());
    PropertyMsgAdaptor::set_assets_table_name(property_options.assets_table_name());
	return true;
}

bool PropertyManager::push(const std::string& gay_name, PropertyMsgAdaptor* propertymsg_adaptor)
{
	// gay_name must be registered
	if (gay_name != "zmkeil") {
		LOG(ERROR, "only zmkeil can push property records");
		return false;
	}

	if (!_pp_operator->push(propertymsg_adaptor)) {
		LOG(ERROR, "push property failed: pp_operate error");
		return false;
	}
	return true;
}

bool PropertyManager::pull(const std::string& gay_name, int begin_index, int max_line, PropertyMsgAdaptor* propertymsg_adaptor)
{
    (void) gay_name;
	if (!_pp_operator->pull(begin_index, max_line, propertymsg_adaptor)) {
        LOG(ERROR, "pull bill failed: pp_operate error");
        return false;
	}
	return true;
}

int PropertyManager::get_last_index() {
	if (!_pp_operator) {
		return 0;
	}
	return _pp_operator->get_last_index();
}

}
