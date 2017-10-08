#include <stdio.h>
#include <iostream>
#include <sstream>
#include <protobuf_util.h>
#include <comlog/info_log_context.h>
#include <controller.h>
#include <channel.h>
#include "bill.pb.h"
#include "propertymsg_adaptor.h"

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

struct PocketContent {
    int year;
    int month;
    std::string comments;
    int money;
    int is_deleted;
};

static PocketContent s_pocket_content[2] = {
    {2017, 9, "xx", 7832, 0},
    {2017, 10, "xx", 9479, 0}
};

struct AssetsContent {
    int year;
    int month;
    int day;
    int store_addr;
    int flow_type;
    int money;
    int store_addr_op;
    int is_deleted;
};

static AssetsContent s_assets_content[2] = {
    {2017, 8, 31, 0, 0, 8000, 0, 0},
    {2017, 9, 30, 1, 0, 5000, 0, 0}
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
	std::cout << "self's bill last index: " << response.last_index() << std::endl;
	int pull_gay_size = (int)response.pull_records().size();
	for (int i = 0; i < pull_gay_size; ++i) {
		const microbill::BillResponse_PullRecords& pull_record = response.pull_records().Get(i);
		std::cout << "update gay: " << pull_record.gay() << std::endl;
		for (int j = 0; j < pull_record.records().size(); j++) { 
			const microbill::Record& record = pull_record.records().Get(j);
			std::cout << microbill::Record_Type_Name(record.type()) << ", " << record.id() << ", " << record.comments() << std::endl;
		}
	}

    //--------------------- property -----------------//
	nrpc::Controller cntl1;
	microbill::PropertyRequest prequest;
	microbill::PropertyResponse presponse;
	prequest.set_gay(gay);
    stub.property(&cntl1, &prequest, &presponse, NULL);
    if (cntl1.Failed()) {
        LOG(ERROR, "%s, property error: \"%s\"", gay.c_str(), cntl1.ErrorText().c_str());
    }
    if (!presponse.status()) {
        LOG(ERROR, "push precords error: \"%s\"", presponse.has_error_msg() ? presponse.error_msg().c_str() : "unkown");
    }
	std::cout << "self's property last index: " << presponse.last_index() << std::endl;

    std::stringstream ss;
    int p_index = presponse.last_index() + 1;
    auto push_record = prequest.mutable_push_property_records();
    for (auto it : s_pocket_content) {
        auto record = push_record->Add();
        record->set_type(microbill::PropertyRecord::NEW);
        record->set_property_type(microbill::PropertyRecord::POCKET_MONEY);
        microbill::PocketRecord* p_record = record->mutable_pocket_record();
        ss.str("");
        ss << p_index++;
        p_record->set_sid(ss.str());
        p_record->set_year(it.year);
        p_record->set_month(it.month);
        p_record->set_comments(it.comments);
        p_record->set_money(it.money);
        p_record->set_is_deleted(it.is_deleted);
    }
    for (auto it : s_assets_content) {
        auto record = push_record->Add();
        record->set_type(microbill::PropertyRecord::NEW);
        record->set_property_type(microbill::PropertyRecord::FIXED_ASSETS);
        microbill::AssetsRecord* a_record = record->mutable_assets_record();
        ss.str("");
        ss << p_index++;
        a_record->set_sid(ss.str());
        a_record->set_year(it.year);
        a_record->set_month(it.month);
        a_record->set_day(it.day);
        a_record->set_store_addr(microbill::s_store_addr_value[it.store_addr]);
        a_record->set_flow_type(microbill::s_flow_type_value[it.flow_type]);
        a_record->set_money(it.money);
        a_record->set_store_addr_op(microbill::s_store_addr_value[it.store_addr_op]);
        a_record->set_is_deleted(it.is_deleted);
    }
    prequest.set_begin_index(1);
	nrpc::Controller cntl2;
    microbill::PropertyResponse presponse2;
    stub.property(&cntl2, &prequest, &presponse2, NULL);
    if (cntl2.Failed()) {
        LOG(ERROR, "%s, property error: \"%s\"", gay.c_str(), cntl2.ErrorText().c_str());
    }
    if (!presponse2.status()) {
        LOG(ERROR, "push precords error: \"%s\"", presponse2.has_error_msg() ? presponse2.error_msg().c_str() : "unkown");
    }
    auto pull_record = presponse2.pull_property_records();
    int pull_size = pull_record.size();
    for (int i = 0; i < pull_size; i++) {
        auto record = pull_record.Get(i);
        std::cout << "action: " << record.type() << ", p_type: " << record.property_type()
                << ", money: " << (record.property_type() == microbill::PropertyRecord::POCKET_MONEY ?
                        record.pocket_record().money() : record.assets_record().money()) << std::endl;
    }

	return 0;
}
