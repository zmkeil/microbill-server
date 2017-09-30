#include <iostream>
#include <gtest/gtest.h>
#include <protobuf_util.h>
#include "bill_manager.h"
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

TEST(BillManagerTest, test_bill_manager_push)
{
	microbill::BillManager bill_manager;
	ASSERT_TRUE(bill_manager.init(env->microbill_config.bill_options(), env->mysql_client));
    microbill::BillMsgAdaptor billmsg_adaptor;

    // gay not register
    ASSERT_FALSE(bill_manager.push("xx", &billmsg_adaptor));

    // push records not setted
    ASSERT_TRUE(bill_manager.push("zmkeil", &billmsg_adaptor));
    ASSERT_EQ(0, bill_manager.get_last_index("zmkeil"));

    // push records is empty
	::google::protobuf::RepeatedPtrField<microbill::Record> push_bill_records;
    billmsg_adaptor.set_push_bill_records(&push_bill_records);
    ASSERT_TRUE(bill_manager.push("zmkeil", &billmsg_adaptor));
    ASSERT_EQ(0, bill_manager.get_last_index("zmkeil"));

    // some records
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

    billmsg_adaptor.set_push_bill_records(&push_bill_records);
    ASSERT_TRUE(bill_manager.push("zmkeil", &billmsg_adaptor));
    ASSERT_STREQ("NEW:zmkeil_2016_03_182| UPDATE:", billmsg_adaptor.push_ids_str().c_str());

    // some other records
    push_bill_records.Clear();
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

    billmsg_adaptor.set_push_bill_records(&push_bill_records);
    ASSERT_TRUE(bill_manager.push("jxj", &billmsg_adaptor));
    ASSERT_STREQ("NEW:jxj_2016_03_32| UPDATE:jxj_2016_03_32|",
            billmsg_adaptor.push_ids_str().c_str());

    ASSERT_EQ(1, bill_manager.get_last_index("zmkeil"));
    ASSERT_EQ(2, bill_manager.get_last_index("jxj"));
}

TEST(BillManagerTest, test_bill_manager_pull)
{
	microbill::BillManager bill_manager;
	ASSERT_TRUE(bill_manager.init(env->microbill_config.bill_options(), env->mysql_client));
    microbill::BillMsgAdaptor billmsg_adaptor;

    // gay not register
    ASSERT_FALSE(bill_manager.pull("xx", 1, 5, NULL));

    // billmsg_adaptor not setted
    ASSERT_TRUE(bill_manager.pull("zmkeil", 1, 5, NULL));

    // pull records not setted
    ASSERT_TRUE(bill_manager.pull("zmkeil", 1, 5, &billmsg_adaptor));

    // pull first
	::google::protobuf::RepeatedPtrField<microbill::Record> pull_bill_records;
    billmsg_adaptor.set_pull_bill_records(&pull_bill_records);
    ASSERT_TRUE(bill_manager.pull("zmkeil", 1, 5, &billmsg_adaptor));
    ASSERT_EQ(1, pull_bill_records.size());
    ASSERT_STREQ("zmkeil_2016_03_182|", billmsg_adaptor.pull_ids_str().c_str());

    const microbill::Record& record = pull_bill_records.Get(0);
    ASSERT_STREQ("zmkeil_2016_03_182", record.id().c_str());
    ASSERT_EQ(24, record.day());
    ASSERT_STREQ("breakfast", record.comments().c_str());

    // pull second
    pull_bill_records.Clear();
    billmsg_adaptor.set_pull_bill_records(&pull_bill_records);
    ASSERT_TRUE(bill_manager.pull("jxj", 1, 5, &billmsg_adaptor));
    ASSERT_EQ(1, pull_bill_records.size());
    ASSERT_STREQ("jxj_2016_03_32|jxj_2016_03_32|", billmsg_adaptor.pull_ids_str().c_str());

    const microbill::Record& record_2 = pull_bill_records.Get(0);
    ASSERT_STREQ("jxj_2016_03_32", record_2.id().c_str());
    ASSERT_EQ(8, record_2.day());
    ASSERT_STREQ("buy two shoes", record_2.comments().c_str());
}
