#include <iostream>
#include <gtest/gtest.h>
#include <protobuf_util.h>
#include "property_manager.h"
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

TEST(PropertyManagerTest, test_property_manager_push)
{
	microbill::PropertyManager property_manager;
	ASSERT_TRUE(property_manager.init(env->microbill_config.property_options(), env->mysql_client));
    microbill::PropertyMsgAdaptor propertymsg_adaptor;

    // only zmkeil can push
    ASSERT_FALSE(property_manager.push("xx", &propertymsg_adaptor));

    // push records not setted
    ASSERT_TRUE(property_manager.push("zmkeil", &propertymsg_adaptor));
    ASSERT_EQ(0, property_manager.get_last_index());

    // push records is empty
	::google::protobuf::RepeatedPtrField<microbill::PropertyRecord> push_property_records;
    propertymsg_adaptor.set_push_property_records(&push_property_records);
    ASSERT_TRUE(property_manager.push("zmkeil", &propertymsg_adaptor));
    ASSERT_EQ(0, property_manager.get_last_index());

    // some records
	microbill::PropertyRecord* record = push_property_records.Add();
	record->set_type(microbill::PropertyRecord::NEW);
	record->set_property_type(microbill::PropertyRecord::POCKET_MONEY);
    microbill::PocketRecord* p_record = record->mutable_pocket_record();
	p_record->set_sid("1");
    p_record->set_year(2017);
    p_record->set_month(9);
    p_record->set_comments("xx");
    p_record->set_money(8788);
    p_record->set_is_deleted(0);

    record = push_property_records.Add();
	record->set_type(microbill::PropertyRecord::NEW);
	record->set_property_type(microbill::PropertyRecord::POCKET_MONEY);
    p_record = record->mutable_pocket_record();
	p_record->set_sid("2");
    p_record->set_year(2017);
    p_record->set_month(10);
    p_record->set_comments("xx");
    p_record->set_money(12923);
    p_record->set_is_deleted(0);

    propertymsg_adaptor.set_push_property_records(&push_property_records);
    ASSERT_TRUE(property_manager.push("zmkeil", &propertymsg_adaptor));
    ASSERT_STREQ("NEW:p_1|p_2| UPDATE:", propertymsg_adaptor.push_ids_str().c_str());
    ASSERT_EQ(2, property_manager.get_last_index());

    // some other records
    push_property_records.Clear();
    record = push_property_records.Add();
	record->set_type(microbill::PropertyRecord::NEW);
	record->set_property_type(microbill::PropertyRecord::FIXED_ASSETS);
    microbill::AssetsRecord* a_record = record->mutable_assets_record();
	a_record->set_sid("1");
    a_record->set_year(2017);
    a_record->set_month(10);
    a_record->set_day(1);
    a_record->set_store_addr(microbill::PropertyRecord_AssetsRecord_StoreAddr_ZM_EBAO);
    a_record->set_flow_type(microbill::PropertyRecord_AssetsRecord_FlowType_STORE);
    a_record->set_money(5000);
    a_record->set_is_deleted(0);

    record = push_property_records.Add();
	record->set_type(microbill::PropertyRecord::UPDATE);
	record->set_property_type(microbill::PropertyRecord::FIXED_ASSETS);
    a_record = record->mutable_assets_record();
	a_record->set_sid("1");
    a_record->set_money(3000);


    propertymsg_adaptor.set_push_property_records(&push_property_records);
    ASSERT_TRUE(property_manager.push("zmkeil", &propertymsg_adaptor));
    ASSERT_STREQ("NEW:a_1| UPDATE:a_1|",
            propertymsg_adaptor.push_ids_str().c_str());
    ASSERT_EQ(4, property_manager.get_last_index());
}

TEST(PropertyManagerTest, test_property_manager_pull)
{
	microbill::PropertyManager property_manager;
	ASSERT_TRUE(property_manager.init(env->microbill_config.property_options(), env->mysql_client));
    microbill::PropertyMsgAdaptor propertymsg_adaptor;

    // propertymsg_adaptor not setted
    ASSERT_TRUE(property_manager.pull("zmkeil", 1, 5, NULL));

    // pull records not setted
    ASSERT_TRUE(property_manager.pull("zmkeil", 1, 5, &propertymsg_adaptor));

    // pull first
	::google::protobuf::RepeatedPtrField<microbill::PropertyRecord> pull_property_records;
    propertymsg_adaptor.set_pull_property_records(&pull_property_records);
    ASSERT_TRUE(property_manager.pull("zmkeil", 1, 5, &propertymsg_adaptor));
    ASSERT_EQ(3, pull_property_records.size());
    ASSERT_STREQ("p_1|p_2|a_1|a_1|", propertymsg_adaptor.pull_ids_str().c_str());

    const microbill::PropertyRecord& record1 = pull_property_records.Get(0);
    ASSERT_EQ(microbill::PropertyRecord::NEW, record1.type());
    ASSERT_EQ(microbill::PropertyRecord::POCKET_MONEY, record1.property_type());
    const microbill::PocketRecord& p_record1 = record1.pocket_record();
    ASSERT_STREQ("1", p_record1.sid().c_str());
    ASSERT_EQ(2017, p_record1.year());
    ASSERT_EQ(9, p_record1.month());
    ASSERT_STREQ("xx", p_record1.comments().c_str());
    ASSERT_EQ(8788, p_record1.money());
    ASSERT_EQ(0, p_record1.is_deleted());

    const microbill::PropertyRecord& record2 = pull_property_records.Get(1);
    ASSERT_EQ(microbill::PropertyRecord::NEW, record2.type());
    ASSERT_EQ(microbill::PropertyRecord::POCKET_MONEY, record2.property_type());
    const microbill::PocketRecord& p_record2 = record2.pocket_record();
    ASSERT_STREQ("2", p_record2.sid().c_str());
    ASSERT_EQ(2017, p_record2.year());
    ASSERT_EQ(10, p_record2.month());
    ASSERT_STREQ("xx", p_record2.comments().c_str());
    ASSERT_EQ(12923, p_record2.money());
    ASSERT_EQ(0, p_record2.is_deleted());

    const microbill::PropertyRecord& record3 = pull_property_records.Get(2);
    ASSERT_EQ(microbill::PropertyRecord::NEW, record3.type());
    ASSERT_EQ(microbill::PropertyRecord::FIXED_ASSETS, record3.property_type());
    const microbill::AssetsRecord& a_record1 = record3.assets_record();
    ASSERT_STREQ("1", a_record1.sid().c_str());
    ASSERT_EQ(2017, a_record1.year());
    ASSERT_EQ(10, a_record1.month());
    ASSERT_EQ(1, a_record1.day());
    ASSERT_EQ(microbill::PropertyRecord_AssetsRecord_StoreAddr_ZM_EBAO, a_record1.store_addr());
    ASSERT_EQ(microbill::PropertyRecord_AssetsRecord_FlowType_STORE, a_record1.flow_type());
    ASSERT_EQ(3000, a_record1.money());
    ASSERT_EQ(microbill::PropertyRecord_AssetsRecord_StoreAddr_ZM_EBAO, a_record1.store_addr_op());
    ASSERT_EQ(0, a_record1.is_deleted());

    // pull second, all gays can pull
    pull_property_records.Clear();
    propertymsg_adaptor.set_pull_property_records(&pull_property_records);
    ASSERT_TRUE(property_manager.pull("jxj", 1, 5, &propertymsg_adaptor));
    ASSERT_EQ(3, pull_property_records.size());
    ASSERT_STREQ("p_1|p_2|a_1|a_1|", propertymsg_adaptor.pull_ids_str().c_str());

    const microbill::PropertyRecord& record_s3 = pull_property_records.Get(2);
    ASSERT_EQ(microbill::PropertyRecord::NEW, record_s3.type());
    ASSERT_EQ(microbill::PropertyRecord::FIXED_ASSETS, record_s3.property_type());
    const microbill::AssetsRecord& a_record_s1 = record_s3.assets_record();
    ASSERT_STREQ("1", a_record_s1.sid().c_str());
    ASSERT_EQ(2017, a_record_s1.year());
    ASSERT_EQ(10, a_record_s1.month());
    ASSERT_EQ(1, a_record_s1.day());
    ASSERT_EQ(microbill::PropertyRecord_AssetsRecord_StoreAddr_ZM_EBAO, a_record_s1.store_addr());
    ASSERT_EQ(microbill::PropertyRecord_AssetsRecord_FlowType_STORE, a_record_s1.flow_type());
    ASSERT_EQ(3000, a_record_s1.money());
    ASSERT_EQ(microbill::PropertyRecord_AssetsRecord_StoreAddr_ZM_EBAO, a_record_s1.store_addr_op());
    ASSERT_EQ(0, a_record_s1.is_deleted());
}
