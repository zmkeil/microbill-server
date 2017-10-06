#include <iostream>
#include <gtest/gtest.h>
#include "propertymsg_adaptor.h"
#include "ut_environment.h"

UTEnvironment* env = NULL;
int main(int argc, char** argv)
{
    /*cases*/
    ::testing::InitGoogleTest(&argc, argv);
	env = new UTEnvironment();
    microbill::PropertyMsgAdaptor::set_pocket_table_name(std::string("pockets"));
    microbill::PropertyMsgAdaptor::set_assets_table_name(std::string("assets"));
	::testing::AddGlobalTestEnvironment(env);
    return RUN_ALL_TESTS();
}

TEST(PropertyMsgAdaptorTest, test_propertymsg_adaptor_push_sqls)
{
    microbill::PropertyMsgAdaptor propertymsg_adaptor;
    ::google::protobuf::RepeatedPtrField<microbill::PropertyRecord> push_property_records;
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

    microbill::EventLines event_lines;
    microbill::SQLs sqls;
    propertymsg_adaptor.set_push_property_records(&push_property_records);
    propertymsg_adaptor.push_sqls(&event_lines, &sqls);
    ASSERT_EQ(4, event_lines.size());
    ASSERT_EQ(4, sqls.size());
    ASSERT_STREQ("NEW:p_1|p_2|a_1| UPDATE:a_1|",
            propertymsg_adaptor.push_ids_str().c_str());

    // event_lines
    microbill::EventLine event_line = event_lines[0];
    ASSERT_STREQ("0", event_line[0].c_str());
    ASSERT_STREQ("p", event_line[1].c_str());
    ASSERT_STREQ("1", event_line[2].c_str());

    event_line = event_lines[1];
    ASSERT_STREQ("0", event_line[0].c_str());
    ASSERT_STREQ("p", event_line[1].c_str());
    ASSERT_STREQ("2", event_line[2].c_str());

    event_line = event_lines[2];
    ASSERT_STREQ("0", event_line[0].c_str());
    ASSERT_STREQ("a", event_line[1].c_str());
    ASSERT_STREQ("1", event_line[2].c_str());

    event_line = event_lines[3];
    ASSERT_STREQ("1", event_line[0].c_str());
    ASSERT_STREQ("a", event_line[1].c_str());
    ASSERT_STREQ("1", event_line[2].c_str());

    // sqls
    microbill::SQL sql = sqls[0];
    ASSERT_EQ(microbill::SQLType::INSERT, sql.first);
    ASSERT_STREQ("insert pockets (sid, year, month, comments, money, is_deleted) VALUES"
            " ('1',2017,9,'xx',8788,0)", sql.second.c_str());
    sql = sqls[1];
    ASSERT_EQ(microbill::SQLType::INSERT, sql.first);
    ASSERT_STREQ("insert pockets (sid, year, month, comments, money, is_deleted) VALUES"
            " ('2',2017,10,'xx',12923,0)", sql.second.c_str());
    sql = sqls[2];
    ASSERT_EQ(microbill::SQLType::INSERT, sql.first);
    ASSERT_STREQ("insert assets (sid, year, month, day, store_addr, flow_type, money, store_addr_op, is_deleted) VALUES"
            " ('1',2017,10,1,0,0,5000,0,0)", sql.second.c_str());
    sql = sqls[3];
    ASSERT_EQ(microbill::SQLType::UPDATE, sql.first);
    ASSERT_STREQ("update assets set money=3000 where sid='1'", sql.second.c_str());
}

TEST(PropertyMsgAdaptorTest, test_propertymsg_adaptor_pull_sqls)
{
    microbill::PropertyMsgAdaptor propertymsg_adaptor;

	microbill::EventLines event_lines;
    microbill::EventLine event_line;
    std::string ev_items[9] = {"1", "p", "1", "0", "p", "2", "0", "a", "1"};
    for (unsigned int i = 0; i < 9; ++i) {
        if (i % 3 == 0 && event_line.size() > 0) {
            event_lines.push_back(event_line);
            event_line.clear();
        }
        event_line.push_back(ev_items[i]);
    }
    if (event_line.size() > 0) {
        event_lines.push_back(event_line);
    }

    microbill::SQLs sqls;
    propertymsg_adaptor.pull_sqls(event_lines, &sqls);
    ASSERT_STREQ("p_1|p_2|a_1|",
            propertymsg_adaptor.pull_ids_str().c_str());

    ASSERT_EQ(2, sqls.size());
    microbill::SQL sql = sqls[0];
    ASSERT_EQ(microbill::SQLType::SELECT, sql.first);
    ASSERT_STREQ("select sid, year, month, comments, money, is_deleted from "
            "pockets where sid in ('1','2')", sql.second.c_str());
    sql = sqls[1];
    ASSERT_EQ(microbill::SQLType::SELECT, sql.first);
    ASSERT_STREQ("select sid, year, month, day, store_addr, flow_type, money, store_addr_op, is_deleted from "
            "assets where sid in ('1')", sql.second.c_str());
}

TEST(PropertyMsgAdaptorTest, test_propertymsg_adaptor_set_pull_records)
{
    microbill::PropertyMsgAdaptor propertymsg_adaptor;

    // init event_lines
	microbill::EventLines event_lines;
    microbill::EventLine event_line;
    std::string ev_items[9] = {"1", "p", "1", "0", "p", "2", "0", "a", "1"};
    for (unsigned int i = 0; i < 9; ++i) {
        if (i % 3 == 0 && event_line.size() > 0) {
            event_lines.push_back(event_line);
            event_line.clear();
        }
        event_line.push_back(ev_items[i]);
    }
    if (event_line.size() > 0) {
        event_lines.push_back(event_line);
    }

    // init record_lines
    microbill::RecordLines record_lines;
    microbill::RecordLine record_line;

	record_line["sid"] = "1";
    record_line["year"] = "2017";
    record_line["month"] = "9";
    record_line["comments"] = "xx";
    record_line["money"] = "8788";
    record_line["is_deleted"] = "0";
    record_lines.push_back(record_line);
    record_line.clear();

	record_line["sid"] = "2";
    record_line["year"] = "2017";
    record_line["month"] = "10";
    record_line["comments"] = "xx";
    record_line["money"] = "12923";
    record_line["is_deleted"] = "0";
    record_lines.push_back(record_line);
    record_line.clear();

	record_line["sid"] = "1";
    record_line["year"] = "2017";
    record_line["month"] = "10";
    record_line["day"] = "1";
    record_line["store_addr"] = "0";
    record_line["flow_type"] = "0";
    record_line["money"] = "3000";
    record_line["store_addr_op"] = "0";
    record_line["is_deleted"] = "0";
    record_lines.push_back(record_line);
    record_line.clear();

    // check set_pull_records
    ::google::protobuf::RepeatedPtrField<microbill::PropertyRecord> pull_property_records;
    propertymsg_adaptor.set_pull_property_records(&pull_property_records);
    propertymsg_adaptor.set_pull_records(event_lines, record_lines);
    ASSERT_EQ(3, pull_property_records.size());

    const microbill::PropertyRecord& record1 = pull_property_records.Get(0);
    ASSERT_EQ(microbill::PropertyRecord::UPDATE, record1.type());
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
}
