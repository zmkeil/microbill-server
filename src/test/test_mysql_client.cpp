#include <iostream>
#include <gtest/gtest.h>
#include "mysql_client.h"
#include "ut_environment.h"

int main(int argc, char** argv)
{
    /*cases*/
    ::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new UTEnvironment());
    return RUN_ALL_TESTS();
}

TEST(MysqlClientTest, test_mysql_client_push_records)
{
	microbill::MysqlClient* client = new microbill::MysqlClient();
	ASSERT_TRUE(client->init());

	microbill::RecordContent contents[2] = {{"zmkeil_2016_03_199", 2016, 3, 12, 0, "zmkeil", "buy electric bike", 2200, 0},{"jxj_2016_03_333", 2016, 3, 15, 0, "jxj", "buy shoes", 328, 0}};
	std::vector<microbill::RecordContent> new_records;
	new_records.push_back(contents[0]);
	new_records.push_back(contents[1]);
	std::vector<microbill::ModifyRecordPair> modify_records;
	ASSERT_TRUE(client->push_records(new_records, modify_records));

	microbill::RecordContent content;
	ASSERT_FALSE(client->query_single_record("zmkeil_2016_03_133", &content));
	ASSERT_TRUE(client->query_single_record("zmkeil_2016_03_199", &content));
	ASSERT_EQ(2200, content.cost);
	ASSERT_STREQ("buy electric bike", content.comments.c_str());

	ASSERT_TRUE(client->query_single_record("jxj_2016_03_333", &content));
	ASSERT_EQ(15, content.day);
	ASSERT_EQ(0, content.pay_earn);
	ASSERT_STREQ("jxj", content.gay.c_str());
	ASSERT_STREQ("buy shoes", content.comments.c_str());
	ASSERT_EQ(328, content.cost);
	ASSERT_EQ(0, content.is_deleted);

	new_records.clear();
	modify_records.push_back(microbill::ModifyRecordPair("cost", "2000"));
	modify_records.push_back(microbill::ModifyRecordPair("id", "'zmkeil_2016_03_199'"));
	ASSERT_TRUE(client->push_records(new_records, modify_records));
	ASSERT_TRUE(client->query_single_record("zmkeil_2016_03_199", &content));
	ASSERT_EQ(2000, content.cost);

	modify_records.clear();
	modify_records.push_back(microbill::ModifyRecordPair("cost", "1800"));
	modify_records.push_back(microbill::ModifyRecordPair("id", "'zmkeil_2016_03_199'"));
	modify_records.push_back(microbill::ModifyRecordPair("comments", "'lunch'"));
	modify_records.push_back(microbill::ModifyRecordPair("cost", "35"));
	modify_records.push_back(microbill::ModifyRecordPair("id", "'jxj_2016_03_333'"));
	ASSERT_TRUE(client->push_records(new_records, modify_records));
	ASSERT_TRUE(client->query_single_record("zmkeil_2016_03_199", &content));
	ASSERT_EQ(1800, content.cost);
	ASSERT_TRUE(client->query_single_record("jxj_2016_03_333", &content));
	ASSERT_EQ(35, content.cost);
	ASSERT_STREQ("lunch", content.comments.c_str());
}

TEST(MysqlClientTest, test_mysql_client_get_records)
{
	microbill::MysqlClient* client = new microbill::MysqlClient();
	ASSERT_TRUE(client->init());

	std::vector<std::string> ids;
	ids.push_back("zmkeil_2016_03_182");
	std::vector<microbill::RecordContent> contents;

	ASSERT_TRUE(client->query_records(ids, &contents));
	ASSERT_EQ(1, contents.size());

	contents.clear();
	ids.push_back("zz_2016_03_12");
	ASSERT_TRUE(client->query_records(ids, &contents));
	ASSERT_EQ(1, contents.size());

	contents.clear();
	ids.push_back("jxj_2016_03_32");
	ASSERT_TRUE(client->query_records(ids, &contents));
	ASSERT_EQ(2, contents.size());

	const microbill::RecordContent& first = contents[0];
	ASSERT_STREQ("zmkeil_2016_03_182", first.id.c_str());
	ASSERT_EQ(12, first.day);

	const microbill::RecordContent& second = contents[1];
	ASSERT_STREQ("jxj_2016_03_32", second.id.c_str());
	ASSERT_EQ(2016, second.year);
	ASSERT_EQ(3, second.month);
	ASSERT_EQ(15, second.day);
	ASSERT_EQ(0, second.pay_earn);
	ASSERT_STREQ("jxj", second.gay.c_str());
	ASSERT_STREQ("buy shoes", second.comments.c_str());
	ASSERT_EQ(328, second.cost);
	ASSERT_EQ(0, second.is_deleted);

	client->close();
}

