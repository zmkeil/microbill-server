#include <iostream>
#include <gtest/gtest.h>
#include "mysql_client.h"
#include "db_helper.h"
#include "ut_environment.h"

int main(int argc, char** argv)
{
    /*cases*/
    ::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new UTEnvironment());
    return RUN_ALL_TESTS();
}

TEST(DBHelperTest, test_db_helper_get_records)
{
	microbill::MysqlClient* client = new microbill::MysqlClient();
	ASSERT_TRUE(client->init());
	microbill::DBHelper db_helper(client);

	microbill::Record* record = NULL;
	std::vector<std::string> ids;
	::google::protobuf::RepeatedPtrField<microbill::Record> updated_records;
	ids.push_back("zmkeil_2016_03_182");
	record = updated_records.Add();
	record->set_id("zmkeil_2016_03_182");
	record->set_type(microbill::Record::NEW);
	ids.push_back("zz_2016_03_12");
	record = updated_records.Add();
	record->set_id("zz_2016_03_12");
	record->set_type(microbill::Record::UPDATE);

	ASSERT_TRUE(db_helper.get_records_by_id_list(ids, &updated_records)); 
	ASSERT_EQ(2, updated_records.size());
	
	const microbill::Record& first = updated_records.Get(0);
	const microbill::Record& second = updated_records.Get(1);

	ASSERT_FALSE(second.has_year());
	ASSERT_TRUE(first.has_year());

	ASSERT_STREQ("zmkeil_2016_03_182", first.id().c_str());
	ASSERT_EQ(2016, first.year());
	ASSERT_EQ(3, first.month());
	ASSERT_EQ(12, first.day());
	ASSERT_EQ(0, first.pay_earn());
	ASSERT_STREQ("zmkeil", first.gay().c_str());
	ASSERT_STREQ("buy electric bike", first.comments().c_str());
	ASSERT_EQ(2200, first.cost());
	ASSERT_EQ(0, first.is_deleted());

	client->close();
}

TEST(DBHelperTest, test_db_helper_push_records)
{
	microbill::MysqlClient* client = new microbill::MysqlClient();
	ASSERT_TRUE(client->init());
	microbill::DBHelper db_helper(client);

	microbill::RecordContent content;
	ASSERT_FALSE(client->query_single_record("zmkeil_2016_03_199", &content));
	ASSERT_TRUE(client->query_single_record("zmkeil_2016_03_182", &content));
	ASSERT_EQ(2200, content.cost);
	ASSERT_TRUE(client->query_single_record("jxj_2016_03_32", &content));
	ASSERT_EQ(328, content.cost);

	microbill::Record* record = NULL;
	::google::protobuf::RepeatedPtrField<microbill::Record> new_records;
	record = new_records.Add();
	record->set_id("zmkeil_2016_03_182");
	record->set_type(microbill::Record::UPDATE);
	record->set_cost(2000);

	record = new_records.Add();
	record->set_id("zmkeil_2016_03_199");
	record->set_type(microbill::Record::NEW);
	record->set_year(2016);
	record->set_month(3);
	record->set_day(18);
	record->set_gay("zmkeil");
	record->set_comments("buy books");
	record->set_cost(88);

	record = new_records.Add();
	record->set_id("jxj_2016_03_32");
	record->set_type(microbill::Record::UPDATE);
	record->set_cost(350);

	ASSERT_TRUE(db_helper.push_records(new_records));

	ASSERT_TRUE(client->query_single_record("zmkeil_2016_03_199", &content));
	ASSERT_STREQ("buy books", content.comments.c_str());
	ASSERT_TRUE(client->query_single_record("zmkeil_2016_03_182", &content));
	ASSERT_EQ(2000, content.cost);
	ASSERT_TRUE(client->query_single_record("jxj_2016_03_32", &content));
	ASSERT_EQ(350, content.cost);
}
