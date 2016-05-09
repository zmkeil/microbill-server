#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include "mysql_client.h"
#include "pending_events.h"
#include "ut_environment.h"

UTEnvironment* env;
int main(int argc, char** argv)
{
    /*cases*/
    ::testing::InitGoogleTest(&argc, argv);
	env = new UTEnvironment();
	::testing::AddGlobalTestEnvironment(env);
    return RUN_ALL_TESTS();
}

static std::string s_id[5] = {"zmkeil_2016_03_21", "jxj_2016_03_15", "jxj_2016_03_32", "zmkeil_2016_03_33", "jxj_2016_03_54"};
static int s_day[5] = {13, 15, 17, 19, 21};
static int s_pay_earn[5] = {0, 0, 1, 0, 1};
static std::string s_gay[5] = {"zmkeil", "jxj", "jxj", "zmkeil", "jxj"};
static int s_cost[5] = {18, 22, 25, 17, 20};

TEST(PendingEventsTest, test_pending_events_get)
{
	// init event from file
	microbill::PendingEvents pending_events("./data/test_events.txt");
	ASSERT_TRUE(pending_events.init());

	// get record from DB with begin_index and max_line
	microbill::MysqlClient* client = new microbill::MysqlClient();
	ASSERT_TRUE(client->init(env->microbill_config.mysql_options()));
	microbill::DBHelper db_helper(client);
	::google::protobuf::RepeatedPtrField<microbill::Record> records;

	ASSERT_TRUE(pending_events.get(1, 3, &db_helper, &records, NULL));
	ASSERT_EQ(3,records.size());

	records.Clear();
   	ASSERT_TRUE(pending_events.get(3, 10, &db_helper, &records, NULL));
	ASSERT_EQ(2,records.size());
   
	records.Clear();
	ASSERT_TRUE(pending_events.get(1, 10, &db_helper, &records, NULL));
	ASSERT_EQ(4,records.size());

	const microbill::Record& record = records.Get(2);
	ASSERT_STREQ("NEW", microbill::Record_Type_Name(record.type()).c_str());
	ASSERT_STREQ("zmkeil_2016_03_182", record.id().c_str());
	ASSERT_TRUE(record.has_year());
	ASSERT_EQ(2016, record.year());
	ASSERT_EQ(3, record.month());
	ASSERT_EQ(12, record.day());
	ASSERT_EQ(0, record.pay_earn());
	ASSERT_STREQ("zmkeil", record.gay().c_str());
	ASSERT_STREQ("buy electric bike", record.comments().c_str());
	ASSERT_EQ(2200, record.cost());
	ASSERT_EQ(0, record.is_deleted());

	// some record don't have content in DB, just return to client
	const microbill::Record& record1 = records.Get(3);
	ASSERT_STREQ("UPDATE", microbill::Record_Type_Name(record1.type()).c_str());
	ASSERT_STREQ("zmkeil_2016_03_12", record1.id().c_str());
	ASSERT_FALSE(record1.has_year());

	const microbill::Record& record2 = records.Get(1);
	ASSERT_STREQ("jxj_2016_03_32", record2.id().c_str());
	ASSERT_TRUE(record2.has_year());
}

TEST(PendingEventsTest, test_pending_events_set)
{
	// init form empty events.txt
	microbill::PendingEvents pending_events("./data/test_events_empty.txt");
	ASSERT_TRUE(pending_events.init());
	::google::protobuf::RepeatedPtrField<microbill::Record> records;

	// set events into file
	for (int i = 0; i < 5; ++i) {
		microbill::Record* record = records.Add();
		record->set_type(microbill::Record::NEW);
		record->set_id(s_id[i]);
		record->set_year(2016);
		record->set_month(3);
		record->set_day(s_day[i]);
		record->set_pay_earn(s_pay_earn[i]);
		record->set_gay(s_gay[i]);
		record->set_comments("lunch");
		record->set_cost(s_cost[i]);
		record->set_is_deleted(0);
	}
	ASSERT_TRUE(pending_events.set(records));


	// then we can get the record from DB
	// the data in DB is just for example
	microbill::MysqlClient* client = new microbill::MysqlClient();
	ASSERT_TRUE(client->init(env->microbill_config.mysql_options()));
	microbill::DBHelper db_helper(client);

	records.Clear();
	ASSERT_TRUE(pending_events.get(1, 8, &db_helper, &records, NULL));
	ASSERT_EQ(5,records.size());

	ASSERT_FALSE(records.Get(0).has_year());
	ASSERT_FALSE(records.Get(1).has_gay());
	ASSERT_FALSE(records.Get(3).has_comments());
	ASSERT_FALSE(records.Get(4).has_cost());

	const microbill::Record& record = records.Get(2);
	ASSERT_TRUE(record.has_day());
	ASSERT_STREQ("NEW", microbill::Record_Type_Name(record.type()).c_str());
	ASSERT_STREQ("jxj_2016_03_32", record.id().c_str());
	ASSERT_EQ(2016, record.year());
	ASSERT_EQ(3, record.month());
	ASSERT_EQ(15, record.day());
	ASSERT_EQ(0, record.pay_earn());
	ASSERT_STREQ("jxj", record.gay().c_str());
	ASSERT_STREQ("buy shoes", record.comments().c_str());
	ASSERT_EQ(328, record.cost());
	ASSERT_EQ(0, record.is_deleted());
}

