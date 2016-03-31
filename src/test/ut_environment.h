#include <fstream>
#include <gtest/gtest.h>
#include <comlog/info_log_context.h>
#include <protobuf_util.h>
#include "mysql_client.h"

class UTEnvironment : public ::testing::Environment {
public:
	UTEnvironment() {}
	virtual ~UTEnvironment() {}

	void SetUp() {
		ASSERT_TRUE(common::load_protobuf_config(&microbill_config));
		
		// truncate the table bills
		microbill::MysqlClient* client = new microbill::MysqlClient();
		ASSERT_TRUE(client->init(microbill_config.mysql_options()));
		client->truncate();

		// insert some data into table bills
		microbill::RecordContent contents[2] = {{"zmkeil_2016_03_182", 2016, 3, 12, 0, "zmkeil", "buy electric bike", 2200, 0},{"jxj_2016_03_32", 2016, 3, 15, 0, "jxj", "buy shoes", 328, 0}};
		std::vector<microbill::RecordContent> new_records;
		new_records.push_back(contents[0]);
		new_records.push_back(contents[1]);
		std::vector<microbill::ModifyRecordPair> modify_records;
		client->push_records(new_records, modify_records);
		client->close();

		// truncate the events.txt (set side)
		std::ofstream ofs;
		ofs.open("./data/test_events_empty.txt", std::ios::out|std::ios::trunc);
		ofs.close();
	}

	void TearDown() {
		return;
	}

	microbill::MicroBillConfig microbill_config;
};
