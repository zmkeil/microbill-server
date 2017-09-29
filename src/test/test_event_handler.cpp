#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include "mysql_client.h"
#include "event_handler.h"
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

TEST(EventHandlerTest, test_event_handler_set)
{
	// init event from file
	microbill::EventHandler event_handler("./data/test_events.txt");
	ASSERT_TRUE(event_handler.init());
    ASSERT_EQ(0, event_handler.get_last_index());

    // set event_lines
    microbill::EventLines event_lines;
    microbill::EventLine event_line;
    std::string start_char = "a";
    int len = 0;
    int loop = 1;
    while (true) {
        event_line.push_back(start_char);
        len++;
        start_char[0]++;
        if (len % loop == 0) {
            event_lines.push_back(event_line);
            event_line.clear();
            loop++;
            len = 0;
        }
        if (loop > 5) {
            break;
        }
    }
    ASSERT_TRUE(event_handler.set(event_lines));
    ASSERT_EQ(5, event_handler.get_last_index());

    // check event_file content
    std::fstream fs;
    fs.open("./data/test_events.txt", std::ios::in);
    char buf[1024] = {0};
    fs.getline(buf, 1024);
    ASSERT_STREQ("1 a", buf);
    fs.getline(buf, 1024);
    ASSERT_STREQ("2 b c", buf);
    fs.getline(buf, 1024);
    ASSERT_STREQ("3 d e f", buf);
    fs.getline(buf, 1024);
    ASSERT_STREQ("4 g h i j", buf);
    fs.getline(buf, 1024);
    ASSERT_STREQ("5 k l m n o", buf);
}

TEST(EventHandlerTest, test_event_handler_get)
{
    microbill::EventHandler event_handler("./data/test_events.txt");
    ASSERT_TRUE(event_handler.init());
    ASSERT_EQ(5, event_handler.get_last_index());

    microbill::EventLines event_lines;
    // begin_index not exist
    ASSERT_FALSE(event_handler.get(0/*begin_index*/, 2/*max_lines*/, &event_lines));

    // begin_index too big
    ASSERT_FALSE(event_handler.get(6/*begin_index*/, 2/*max_lines*/, &event_lines));

    // == max_lines
    ASSERT_TRUE(event_handler.get(2/*begin_index*/, 2/*max_lines*/, &event_lines));
    ASSERT_EQ(2, event_lines.size());

    // < max_lines
    ASSERT_TRUE(event_handler.get(5/*begin_index*/, 2/*max_lines*/, &event_lines));
    ASSERT_EQ(2/*already got*/ + 1, event_lines.size());

    microbill::EventLine event_line = event_lines[0];
    ASSERT_STREQ("b", event_line[0].c_str());
    event_line = event_lines[1];
    ASSERT_STREQ("d", event_line[0].c_str());
    event_line = event_lines[2];
    ASSERT_STREQ("k", event_line[0].c_str());
    ASSERT_STREQ("l", event_line[1].c_str());
    ASSERT_STREQ("m", event_line[2].c_str());
    ASSERT_STREQ("n", event_line[3].c_str());
    ASSERT_STREQ("o", event_line[4].c_str());
}

