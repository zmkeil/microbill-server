#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include <comlog/info_log_context.h>
#include <protobuf_util.h>
#include "db_handler.h"
#include "mysql_client.h"

class UTEnvironment : public ::testing::Environment {
public:
	microbill::MicroBillConfig microbill_config;
    microbill::MysqlClient* mysql_client;
    std::string table_bill;
    std::string table_pocket;
    std::string table_assets;

    std::string inset_pocket_prefix;
    std::string update_pocket_prefix;
    std::string select_pocket_prefix;

public:
	UTEnvironment() {}
	virtual ~UTEnvironment() {}

	void SetUp() {
		ASSERT_TRUE(common::load_protobuf_config(&microbill_config));

		// truncate the table bills
		mysql_client = new microbill::MysqlClient();
		ASSERT_TRUE(mysql_client->init(microbill_config.mysql_options()));

        microbill::DBHandler db_handler(mysql_client);
		table_bill = microbill_config.bill_options().table_name();
        table_pocket = microbill_config.property_options().pocket_table_name();
        table_assets = microbill_config.property_options().assets_table_name();
		db_handler.truncate(table_bill);
		db_handler.truncate(table_pocket);
		db_handler.truncate(table_assets);

        // some sql prefix
        inset_pocket_prefix = "insert " + table_pocket
                + "(sid, year, month, money) values ";
        update_pocket_prefix = "update " + table_pocket + " ";
        select_pocket_prefix = "select * from " + table_pocket + " ";

		// truncate the events.txt (set side)
		std::ofstream ofs;
		ofs.open("./data/zmkeil_events.txt", std::ios::out|std::ios::trunc);
		ofs.close();
		ofs.open("./data/jxj_events.txt", std::ios::out|std::ios::trunc);
		ofs.close();
		ofs.open("./data/test_events.txt", std::ios::out|std::ios::trunc);
		ofs.close();
	}

	void TearDown() {
        mysql_client->close();
		return;
	}
};
