#include <iostream>
#include <gtest/gtest.h>
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

TEST(BillMsgAdaptorTest, test_billmsg_adaptor_push_sqls)
{
    microbill::BillMsgAdaptor billmsg_adaptor;
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

    microbill::EventLines event_lines;
    microbill::SQLs sqls;
    billmsg_adaptor.set_push_bill_records(&push_bill_records);
    billmsg_adaptor.push_sqls(&event_lines, &sqls);
    ASSERT_EQ(3, event_lines.size());
    ASSERT_EQ(3, sqls.size());
    ASSERT_STREQ("NEW:zmkeil_2016_03_182|jxj_2016_03_32| UPDATE:jxj_2016_03_32|",
            billmsg_adaptor.push_ids_str().c_str());

    // event_lines
    microbill::EventLine event_line = event_lines[0];
    ASSERT_STREQ("0", event_line[0].c_str());
    ASSERT_STREQ("zmkeil_2016_03_182", event_line[1].c_str());

    event_line = event_lines[1];
    ASSERT_STREQ("0", event_line[0].c_str());
    ASSERT_STREQ("jxj_2016_03_32", event_line[1].c_str());

    event_line = event_lines[2];
    ASSERT_STREQ("1", event_line[0].c_str());
    ASSERT_STREQ("jxj_2016_03_32", event_line[1].c_str());

    // sqls
    microbill::SQL sql = sqls[0];
    ASSERT_EQ(microbill::SQLType::INSERT, sql.first);
    ASSERT_STREQ("insert bills (id, year, month, day, pay_earn, gay, comments, cost, is_deleted) VALUES"
            " ('zmkeil_2016_03_182',2016,3,24,0,'zmkeil','breakfast',9,0)", sql.second.c_str());
    sql = sqls[1];
    ASSERT_EQ(microbill::SQLType::INSERT, sql.first);
    ASSERT_STREQ("insert bills (id, year, month, day, pay_earn, gay, comments, cost, is_deleted) VALUES"
            " ('jxj_2016_03_32',2016,3,7,0,'jxj','buy shoes',168,0)", sql.second.c_str());
    sql = sqls[2];
    ASSERT_EQ(microbill::SQLType::UPDATE, sql.first);
    ASSERT_STREQ("update bills set day=8,comments='buy two shoes' where id='jxj_2016_03_32'", sql.second.c_str());
}

TEST(BillMsgAdaptorTest, test_billmsg_adaptor_pull_sqls)
{
    microbill::BillMsgAdaptor billmsg_adaptor;

	microbill::EventLines event_lines;
    microbill::EventLine event_line;
    std::string ev_items[6] = {"1", "zmkeil_2016_03_182", "0", "jxj_2016_03_32", "1", "jxj_2016_03_32"};
    for (unsigned int i = 0; i < 6; ++i) {
        if (i % 2 == 0 && event_line.size() > 0) {
            event_lines.push_back(event_line);
            event_line.clear();
        }
        event_line.push_back(ev_items[i]);
    }
    if (event_line.size() > 0) {
        event_lines.push_back(event_line);
    }

    microbill::SQLs sqls;
    billmsg_adaptor.pull_sqls(event_lines, &sqls);
    ASSERT_STREQ("zmkeil_2016_03_182|jxj_2016_03_32|jxj_2016_03_32|",
            billmsg_adaptor.pull_ids_str().c_str());

    ASSERT_EQ(1, sqls.size());
    microbill::SQL sql = sqls[0];
    ASSERT_EQ(microbill::SQLType::SELECT, sql.first);
    ASSERT_STREQ("select id, year, month, day, pay_earn, gay, comments, cost, is_deleted from "
            "bills where id in ('zmkeil_2016_03_182','jxj_2016_03_32','jxj_2016_03_32')", sql.second.c_str());
}

TEST(BillMsgAdaptorTest, test_billmsg_adaptor_set_pull_records)
{
    microbill::BillMsgAdaptor billmsg_adaptor;

    // init event_lines
	microbill::EventLines event_lines;
    microbill::EventLine event_line;
    std::string ev_items[6] = {"1", "zmkeil_2016_03_182", "0", "jxj_2016_03_32", "1", "jxj_2016_03_32"};
    for (unsigned int i = 0; i < 6; ++i) {
        if (i % 2 == 0 && event_line.size() > 0) {
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

	record_line["id"] = "zmkeil_2016_03_182";
    record_line["year"] = "2016";
    record_line["month"] = "3";
    record_line["day"] = "24";
    record_line["pay_earn"] = "0";
    record_line["gay"] = "zmkeil";
    record_line["comments"] = "breakfast";
    record_line["cost"] = "9";
    record_line["is_deleted"] = "0";
    record_lines.push_back(record_line);
    record_line.clear();

	record_line["id"] = "jxj_2016_03_32";
    record_line["year"] = "2016";
    record_line["month"] = "3";
    record_line["day"] = "8";
    record_line["pay_earn"] = "0";
    record_line["gay"] = "jxj";
    record_line["comments"] = "buy two shoes";
    record_line["cost"] = "168";
    record_line["is_deleted"] = "0";
    record_lines.push_back(record_line);
    record_line.clear();

    // check set_pull_records
    ::google::protobuf::RepeatedPtrField<microbill::Record> pull_bill_records;
    billmsg_adaptor.set_pull_bill_records(&pull_bill_records);
    billmsg_adaptor.set_pull_records(event_lines, record_lines);
    ASSERT_EQ(2, pull_bill_records.size());

    const microbill::Record& record1 = pull_bill_records.Get(0);
    ASSERT_STREQ("zmkeil_2016_03_182", record1.id().c_str());
    ASSERT_EQ(microbill::Record::UPDATE, record1.type());
    ASSERT_EQ(2016, record1.year());
    ASSERT_EQ(3, record1.month());
    ASSERT_EQ(24, record1.day());
    ASSERT_EQ(0, record1.pay_earn());
    ASSERT_STREQ("zmkeil", record1.gay().c_str());
    ASSERT_STREQ("breakfast", record1.comments().c_str());
    ASSERT_EQ(9, record1.cost());
    ASSERT_EQ(0, record1.is_deleted());

    const microbill::Record& record2 = pull_bill_records.Get(1);
    ASSERT_STREQ("jxj_2016_03_32", record2.id().c_str());
    ASSERT_EQ(microbill::Record::NEW, record2.type());
    ASSERT_STREQ("buy two shoes", record2.comments().c_str());
    ASSERT_EQ(168, record2.cost());
}
