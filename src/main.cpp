#include <protobuf_util.h>
#include <comlog/info_log_context.h>
#include <server.h>
#include <controller.h>
#include "bill.pb.h"
#include "db_helper.h"
#include "mysql_client.h"
#include "user_manager.h"
#include "bill_context.h"
#include "bill_service_impl.h"
#include "util.h"

int main()
{
	if (!sample::server_side_config_log()) {
		LOG(ALERT, "Failed to config log");
		return -1;
	}

	microbill::MicroBillConfig microbill_config;
	if (!common::load_protobuf_config(&microbill_config)) {
		LOG(ALERT, "Failed to parse the conf/startup/service.conf");
		return -1;
	}
		
	microbill::MysqlClient* mysql_client = new microbill::MysqlClient();
	if (!mysql_client->init(microbill_config.mysql_options())) {
		LOG(ALERT, "Failed to init mysql");
		return -1;
	}
	microbill::DBHelper* db_helper = new microbill::DBHelper(mysql_client);

	microbill::UserManager* user_manager = new microbill::UserManager();
	if (!user_manager->init(microbill_config.user_options())) {
		LOG(ALERT, "Failed to init user_manager");
		return -1;
	}

	const microbill::ServerOptions& server_config = microbill_config.server_options();
	nrpc::Server server;
	microbill::BillServiceImpl service;
	nrpc::ServiceSet* service_set = server.push_service_set(server_config.local_address());
    service_set->add_service(&service);

    nrpc::ServerOption option;
	microbill::BillContextFactory context_factory(db_helper, user_manager);
	option.service_context_factory = &context_factory;
    option.is_connection_reuse = server_config.connection_reuse();
    option.idle_timeout = server_config.idle_time();
    server.start(&option);

	return 0;
}

