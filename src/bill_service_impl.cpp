#include <iostream>
#include <comlog/info_log_context.h>
#include <controller.h>
#include "bill.pb.h"
#include "mysql_client.h"
#include "billmsg_adaptor.h"
#include "bill_manager.h"
#include "bill_context.h"
#include "bill_service_impl.h"

namespace microbill {

void BillServiceImpl::update(google::protobuf::RpcController* cntl_base,
			const BillRequest* request,
			BillResponse* response,
			google::protobuf::Closure* done)
{
	nrpc::Controller* cntl = static_cast<nrpc::Controller*>(cntl_base);
	BillContext* context = static_cast<BillContext*>(cntl->service_context());
	BillManager* bill_manager = context->bill_manager;
    BillMsgAdaptor billmsg_adaptor;
	context->set_session_field("gay", request->gay());

	// 1. first set events for others and don't push DB if failed, so gurante others
	// can get your update.
	// 2. set response->status = false, so client will retry later
	// 3. set response->status = false if push DB failed
	// 4. don't worry about the events and records already set(insert) before failed, 
	// 	    for events: other client will deal with the Duplicate events;
	//	 	for records: server will deal with the Duplicate events; 
    std::string gay = request->gay();
	response->set_last_index(bill_manager->get_last_index(gay));
    bool ret = true;
	if (request->push_records().size() > 0) {
        billmsg_adaptor.set_push_bill_records(&(request->push_records()));
		if (!bill_manager->push(request->gay(), &billmsg_adaptor)) {
            ret = false;
			response->set_error_msg("push bill records failed");
		}
    }
    response->set_status(ret);
    context->set_session_field("push_records", billmsg_adaptor.push_ids_str());
	context->set_session_field("status", ret ? "true" : "false");
    if (!ret) {
        context->set_session_field("err_msg", response->error_msg());
    }


	// 1. server side don't remember which events you have already get, it just return
	// records begin from request->begin_index()
	// 2. client side deal with data integrity, which means that it will retry if the last results is suspicious
    std::string all_pull_ids;
	for (int i = 0; i < request->pull_infos().size(); i++) {
		const BillRequest_PullInfo& pull_info = request->pull_infos().Get(i);
		std::string gay = pull_info.gay();
		int begin_index = pull_info.begin_index();
		int max_line = pull_info.max_line();
		BillResponse_PullRecords* pull_record = response->add_pull_records();
		pull_record->set_gay(gay);
		::google::protobuf::RepeatedPtrField<Record>* records = pull_record->mutable_records();
        billmsg_adaptor.set_pull_bill_records(records);
		if (!bill_manager->pull(gay, begin_index, max_line, &billmsg_adaptor)) {
			LOG(WARN, "pull %s's records failed", gay.c_str());
		}
        all_pull_ids += gay + ":" + billmsg_adaptor.pull_ids_str() + " ";
	}
    context->set_session_field("pull_records", all_pull_ids);

	return done->Run();
}

}
