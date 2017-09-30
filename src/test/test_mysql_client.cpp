#include <iostream>
#include <gtest/gtest.h>
#include "mysql_client.h"
#include "ut_environment.h"

UTEnvironment* env = NULL;
int main(int argc, char** argv)
{
    /*cases*/
    ::testing::InitGoogleTest(&argc, argv);
	env = new UTEnvironment();
	::testing::AddGlobalTestEnvironment(env);
    return RUN_ALL_TESTS();
}

TEST(MysqlClientTest, test_mysql_client_insert)
{
	microbill::MysqlClient* client = env->mysql_client;
    microbill::DBHandler db_handler(client);

    // insert some data into table pocket
    microbill::SQLs sqls;
    std::string pocket_values[] = {"(1, 2017, 8, 7323), (2, 2017, 9, 7596)",
            "(2, 2017, 8, 7323)", "(3, 2017, 9, 7596)"};
    unsigned int len = sizeof(pocket_values) / sizeof(pocket_values[0]);
    for (unsigned int i = 0; i < len; ++i) {
        std::string sql_str = env->inset_pocket_prefix + pocket_values[i];
        microbill::SQL sql = std::make_pair(microbill::SQLType::INSERT, sql_str);
        sqls.push_back(sql);
    }
    ASSERT_TRUE(db_handler.push_records(sqls));

    // duplicate entry
    std::string sql_str = env->inset_pocket_prefix + "(3, 2017, 9, 7588)";
    microbill::SQL sql = std::make_pair(microbill::SQLType::INSERT, sql_str);
    ASSERT_TRUE(client->query(sql, NULL));

    // error entry
    sql_str = env->inset_pocket_prefix + "(4, 2017, 9, 7588, 10)";
    sql = std::make_pair(microbill::SQLType::INSERT, sql_str);
    ASSERT_FALSE(client->query(sql, NULL));

    // correct entry
    sql_str = env->inset_pocket_prefix + "(4, 2017, 9, 7588)";
    sql = std::make_pair(microbill::SQLType::INSERT, sql_str);
    ASSERT_TRUE(client->query(sql, NULL));
}

TEST(MysqlClientTest, test_mysql_client_update)
{
	microbill::MysqlClient* client = env->mysql_client;

    // not exist field
    std::string sql_str = env->update_pocket_prefix + "set sb='sb' where sid='4'";
    microbill::SQL sql = std::make_pair(microbill::SQLType::UPDATE, sql_str);
    ASSERT_FALSE(client->query(sql, NULL));

    // not exist id
    sql_str = env->update_pocket_prefix + "set money=8000 where sid='10'";
    sql = std::make_pair(microbill::SQLType::UPDATE, sql_str);
    ASSERT_TRUE(client->query(sql, NULL));

    // not exist id
    sql_str = env->update_pocket_prefix + "set money=8000 where sid='4'";
    sql = std::make_pair(microbill::SQLType::UPDATE, sql_str);
    ASSERT_TRUE(client->query(sql, NULL));
}

TEST(MysqlClientTest, test_mysql_client_select)
{
	microbill::MysqlClient* client = env->mysql_client;

    // null record_lines
    std::string sql_str = env->select_pocket_prefix + "where sid in ('1','2','3','4','5')";
    microbill::SQL sql = std::make_pair(microbill::SQLType::SELECT, sql_str);
    ASSERT_FALSE(client->query(sql, NULL));

    // get 4 records
    microbill::RecordLines record_lines;
    ASSERT_TRUE(client->query(sql, &record_lines));
	ASSERT_EQ(4, record_lines.size());

    // check record
    // not update
    microbill::RecordLine record_line = record_lines[0];
    ASSERT_TRUE(record_line.find("sid") != record_line.end());
    ASSERT_STREQ("1", record_line["sid"].c_str());
    ASSERT_TRUE(record_line.find("money") != record_line.end());
    ASSERT_STREQ("7323", record_line["money"].c_str());

    // duplicate insert but not update
    record_line = record_lines[2];
    ASSERT_TRUE(record_line.find("sid") != record_line.end());
    ASSERT_STREQ("3", record_line["sid"].c_str());
    ASSERT_TRUE(record_line.find("money") != record_line.end());
    ASSERT_STREQ("7596", record_line["money"].c_str());

    // update
    record_line = record_lines[3];
    ASSERT_TRUE(record_line.find("sid") != record_line.end());
    ASSERT_STREQ("4", record_line["sid"].c_str());
    ASSERT_TRUE(record_line.find("money") != record_line.end());
    ASSERT_STREQ("8000", record_line["money"].c_str());

    // append records
    sql_str = env->select_pocket_prefix + "where sid in ('1','2')";
    sql = std::make_pair(microbill::SQLType::SELECT, sql_str);
    ASSERT_TRUE(client->query(sql, &record_lines));
	ASSERT_EQ(4/*old*/ + 2, record_lines.size());
}

