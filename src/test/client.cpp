#include <stdio.h>
#include <iostream>
#include <protobuf_util.h>
#include <comlog/info_log_context.h>
#include <controller.h>
#include <channel.h>
#include "db_helper.h"
#include "bill.pb.h"

static microbill::RecordContent s_contents[3] = {
	{"_2016_03_18", 2016, 3, 15, 0, "gay", "lunch", 55, 0},
	{"_2016_03_36", 2016, 3, 19, 0, "gay", "shoes", 328, 0},
	{"_2016_03_44", 2016, 3, 28, 0, "gay", "play", 100, 0}
};

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage: ./client <gay>\n");
		return -1;
	}
	std::string gay(argv[1]);

	nrpc::Channel channel;
	nrpc::ChannelOption option;
	if (!channel.init("127.0.0.1", 8844, &option)) {
		LOG(ERROR, "Fail to init channel");
		return -1;
	}

	microbill::BillService_Stub stub(&channel);
	nrpc::Controller cntl;
	microbill::BillRequest request;
	microbill::BillResponse response;

	request.set_gay(gay);
	for (int i = 0; i < 3; ++i) {
		microbill::Record* record = request.mutable_records()->Add();
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
	request.set_begin_index(1);
	request.set_max_line(10);
	stub.update(&cntl, &request, &response, NULL);

	if (cntl.Failed()) {
		LOG(ERROR, "%s, update error: \"%s\"", gay.c_str(), cntl.ErrorText().c_str());
	}
	if (response.status()) {
		LOG(ERROR, "push records error: \"%s\"", response.has_error_msg() ? response.error_msg().c_str() : "unkown");
	}
	int record_size = (int)response.records().size();
	for (int i = 0; i < record_size; ++i) {
		const microbill::Record& record = response.records().Get(i);
		std::cout << microbill::Record_Type_Name(record.type()) << ", " << record.id() << ", " << record.comments() << std::endl;
	}

	return 0;
}
