#include <comlog/info_log_context.h>
#include <controller.h>
#include "bill.pb.h"
#include "db_helper.h"
#include "user_manager.h"
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
	DBHelper* db_helper = context->db_helper;
	UserManager* user_manager = context->user_manager;
	context->set_session_field("gay", request->gay());

	// 1. first set events for others and don't push DB if failed, so gurante others
	// can get your update.
	// 2. set response->status = false, so client will retry later
	// 3. set response->status = false if push DB failed
	// 4. don't worry about the events and records already set(insert) before failed, 
	// 	    for events: other client will deal with the Duplicate events;
	//	 	for records: server will deal with the Duplicate events; 
	response->set_last_index(user_manager->get_last_index(request->gay()));
	response->set_status(true);
	if (request->push_records().size() > 0) {
		if (!user_manager->set_events(request->gay(), request->push_records(), context)) {
			response->set_status(false);
			response->set_error_msg("Fail to set events in PendingEvent");
		} else {
			if (!db_helper->push_records(request->push_records(), context)) {
				response->set_status(false);
				response->set_error_msg("Fail to push records into DB");
			}
		}
	}
	context->set_session_field("status", response->status() ? "true" : "false");
	if (!response->status()) {
		context->set_session_field("err_msg", response->has_error_msg() ? response->error_msg() : "unkown");
	}

	// 1. server side don't remember which events you have already get, it just return 
	// records begin from request->begin_index()
	// 2. client side deal with data integrity, which means that it will retry if the last results is suspicious
	for (int i = 0; i < request->pull_infos().size(); i++) {
		const BillRequest_PullInfo& pull_info = request->pull_infos().Get(i);
		std::string gay = pull_info.gay();
		int begin_index = pull_info.begin_index();
		int max_line = pull_info.max_line();
		BillResponse_PullRecords* pull_record = response->add_pull_records();
		pull_record->set_gay(gay);
		::google::protobuf::RepeatedPtrField<Record>* records = pull_record->mutable_records();
		if (!user_manager->get_events(gay, begin_index, max_line,
				db_helper, records, context)) {
			LOG(DEBUG, "no more events for %s", gay.c_str());
		}
	}

	return done->Run();
}

}
