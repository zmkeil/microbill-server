#include <iostream>
#include <gtest/gtest.h>
#include "bill.pb.h"
#include "pp_operator.h"
#include "billmsg_adaptor.h"
#include "ut_environment.h"

UTEnvironment* env = NULL;
int main(int argc, char** argv)
{
    /*cases*/
    ::testing::InitGoogleTest(&argc, argv);
	env = new UTEnvironment();
    microbill::BillMsgAdaptor::set_bill_table_name(std::string("bills"));
	::testing::AddGlobalTestEnvironment(env);
    return RUN_ALL_TESTS();
}

TEST(PPOperatorTest, test_pp_operator_push)
{
	microbill::MysqlClient* client = env->mysql_client;
    microbill::DBHandler db_handler(client);
    microbill::EventHandler event_handler("./data/test_events.txt");
    microbill::PPOperator pp_operator(&event_handler, &db_handler);
    ASSERT_TRUE(pp_operator.init());

    ::google::protobuf::RepeatedPtrField<microbill::Record> push_bill_records;
	microbill::Record* record = push_bill_records.Add();
	record->set_id("zmkeil_2016_03_182");
	record->set_type(microbill::Record::NEW);
    record->set_year(2016);
    record->set_month(3);
    record->set_day(24);
    record->set_pay_earn(0);
    record->set_gay("zmkeil");
    record->set_comments("breakfast");
    record->set_cost(9);

	record = push_bill_records.Add();
	record->set_id("jxj_2016_03_32");
	record->set_type(microbill::Record::NEW);
    record->set_year(2016);
    record->set_month(3);
    record->set_day(7);
    record->set_pay_earn(0);
    record->set_gay("jxj");
    record->set_comments("buy shoes");
    record->set_cost(168);

	record = push_bill_records.Add();
	record->set_id("jxj_2016_03_32");
	record->set_type(microbill::Record::UPDATE);
    record->set_day(8);
    record->set_comments("buy two shoes");

    microbill::BillMsgAdaptor billmsg_adaptor(push_bill_records, NULL);
    ASSERT_TRUE(pp_operator.push(&billmsg_adaptor));
    ASSERT_EQ(3, pp_operator.get_last_index());
}

TEST(PPOperatorTest, test_pp_operator_pull)
{
	microbill::MysqlClient* client = env->mysql_client;
    microbill::DBHandler db_handler(client);
    microbill::EventHandler event_handler("./data/test_events.txt");
    microbill::PPOperator pp_operator(&event_handler, &db_handler);
    ASSERT_TRUE(pp_operator.init());
    ASSERT_EQ(3, pp_operator.get_last_index());

    ::google::protobuf::RepeatedPtrField<microbill::Record> push_bill_records;
    ::google::protobuf::RepeatedPtrField<microbill::Record> pull_bill_records;
    microbill::BillMsgAdaptor billmsg_adaptor(push_bill_records, &pull_bill_records);

    ASSERT_TRUE(pp_operator.pull(1/*begin_index*/, 5/*max_lines*/, &billmsg_adaptor));
    ASSERT_EQ(2, pull_bill_records.size());

    // check pull records
    microbill::Record record = pull_bill_records.Get(1);
    ASSERT_STREQ("zmkeil_2016_03_182", record.id().c_str());
    ASSERT_EQ(9, record.cost());

    record = pull_bill_records.Get(0);
    ASSERT_STREQ("jxj_2016_03_32", record.id().c_str());
    ASSERT_STREQ("buy two shoes", record.comments().c_str());
    ASSERT_EQ(8, record.day());
    ASSERT_EQ(168, record.cost());
}
