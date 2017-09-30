#include <iostream>
#include <protobuf_util.h>
#include <comlog/info_log_context.h>
#include <server.h>
#include <controller.h>
#include "bill.pb.h"
#include "mysql_client.h"
#include "bill_manager.h"
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

	microbill::BillManager* bill_manager = new microbill::BillManager();
	if (!bill_manager->init(microbill_config.bill_options(), mysql_client)) {
		LOG(ALERT, "Failed to init bill_manager");
		return -1;
	}
    std::string user1 = "zmkeil";
    std::cout << user1 << " last_index: " << bill_manager->get_last_index(user1) << std::endl;

	const microbill::ServerOptions& server_config = microbill_config.server_options();
	nrpc::Server server;
	microbill::BillServiceImpl service;
	nrpc::ServiceSet* service_set = server.push_service_set(server_config.local_address());
    service_set->add_service(&service);

    nrpc::ServerOption option;
	microbill::BillContextFactory context_factory(bill_manager);
	option.service_context_factory = &context_factory;
    option.is_connection_reuse = server_config.connection_reuse();
    option.idle_timeout = server_config.idle_time();
    server.start(&option);

	return 0;
}

