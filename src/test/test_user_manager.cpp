#include <iostream>
#include <gtest/gtest.h>
#include <protobuf_util.h>
#include "db_helper.h"
#include "user_manager.h"
#include "ut_environment.h"

int main(int argc, char** argv)
{
    /*cases*/
    ::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new UTEnvironment());
    return RUN_ALL_TESTS();
}

TEST(UserManagerTest, test_user_manager_get_events_for_self)
{
	microbill::MicroBillConfig microbill_config;
	ASSERT_TRUE(common::load_protobuf_config(&microbill_config));
	microbill::UserManager user_manager;
	ASSERT_TRUE(user_manager.init(microbill_config.user_options()));

	microbill::MysqlClient* client = new microbill::MysqlClient();
	ASSERT_TRUE(client->init());
	microbill::DBHelper db_helper(client);
	::google::protobuf::RepeatedPtrField<microbill::Record> records;

	ASSERT_TRUE(user_manager.get_events_for_self("zmkeil", 1, 10, &db_helper, &records));
	ASSERT_EQ(3, records.size());
	ASSERT_FALSE(records.Get(0).has_year());

	const microbill::Record& record = records.Get(1);
	ASSERT_TRUE(record.has_year());
	ASSERT_EQ(328, record.cost());
	ASSERT_STREQ("buy shoes", record.comments().c_str());
}

TEST(UserManagerTest, test_user_manager_set_events_for_others)
{
	microbill::MicroBillConfig microbill_config;
	ASSERT_TRUE(common::load_protobuf_config(&microbill_config));
	microbill::UserManager user_manager;
	ASSERT_TRUE(user_manager.init(microbill_config.user_options()));

	::google::protobuf::RepeatedPtrField<microbill::Record> records;
	microbill::Record* record = records.Add();
	record->set_type(microbill::Record::NEW);
	record->set_id("zmkeil_2016_03_544");
	record = records.Add();
	record->set_type(microbill::Record::UPDATE);
	record->set_id("zmkeil_2016_03_566");

	ASSERT_TRUE(user_manager.set_events_for_others("zmkeil", records));
}
