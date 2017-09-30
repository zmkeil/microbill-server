#include <stdio.h>
#include <iostream>
#include <protobuf_util.h>
#include <comlog/info_log_context.h>
#include <controller.h>
#include <channel.h>
#include "bill.pb.h"

struct RecordContent {
    std::string id;
    int year;
    int month;
    int day;
    int pay_earn;
    std::string gay;
    std::string comments;
    int cost;
    int is_deleted;
};

static RecordContent s_contents[3] = {
	{"_2016_03_18", 2016, 3, 15, 0, "gay", "lunch", 55, 0},
	{"_2016_03_36", 2016, 3, 19, 0, "gay", "shoes", 328, 0},
	{"_2016_03_44", 2016, 3, 28, 0, "gay", "play", 100, 0}
};

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Usage: ./client gay <sync_gay>\n");
		return -1;
	}
	std::string gay(argv[1]);

	nrpc::Channel channel;
	nrpc::ChannelOption option;
	if (!channel.init("127.0.0.1", 8888, &option)) {
		LOG(ERROR, "Fail to init channel");
		return -1;
	}

	microbill::BillService_Stub stub(&channel);
	nrpc::Controller cntl;
	microbill::BillRequest request;
	microbill::BillResponse response;

	request.set_gay(gay);
	for (int i = 0; i < 3; ++i) {
		microbill::Record* record = request.mutable_push_records()->Add();
		record->set_type(microbill::Record::NEW);
		record->set_id(gay + s_contents[i].id);
		record->set_year(s_contents[i].year);
		record->set_month(s_contents[i].month);
		record->set_day(s_contents[i].day);
		record->set_pay_earn(s_contents[i].pay_earn);
		record->set_gay(gay);
		record->set_comments(s_contents[i].comments);
		record->set_cost(s_contents[i].cost);
		record->set_is_deleted(s_contents[i].is_deleted);
	}

	for (int i = 2; i < argc; i++) {
		std::string sync_gay(argv[i]);
		microbill::BillRequest_PullInfo* pull_info = request.add_pull_infos();
		pull_info->set_gay(sync_gay);
		pull_info->set_begin_index(1);
		pull_info->set_max_line(10);
	}

	stub.update(&cntl, &request, &response, NULL);
	if (cntl.Failed()) {
		LOG(ERROR, "%s, update error: \"%s\"", gay.c_str(), cntl.ErrorText().c_str());
	}
	if (!response.status()) {
		LOG(ERROR, "push records error: \"%s\"", response.has_error_msg() ? response.error_msg().c_str() : "unkown");
	}
	std::cout << "self's last index: " << response.last_index() << std::endl;
	int pull_gay_size = (int)response.pull_records().size();
	for (int i = 0; i < pull_gay_size; ++i) {
		const microbill::BillResponse_PullRecords& pull_record = response.pull_records().Get(i);
		std::cout << "update gay: " << pull_record.gay() << std::endl;
		for (int j = 0; j < pull_record.records().size(); j++) { 
			const microbill::Record& record = pull_record.records().Get(j);
			std::cout << microbill::Record_Type_Name(record.type()) << ", " << record.id() << ", " << record.comments() << std::endl;
		}
	}

	return 0;
}
