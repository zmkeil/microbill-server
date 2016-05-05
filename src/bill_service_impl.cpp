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
	db_helper->set_context(context);
	UserManager* user_manager = context->user_manager;
	user_manager->set_context(context);

	// 1. first set events for others and don't push DB if failed, so gurante others
	// can get your update.
	// 2. set response->status = false, so client will retry later
	// 3. set response->status = false if push DB failed
	// 4. don't worry about the events and records already set(insert) before failed, 
	// 	    for events: other client will deal with the Duplicate events;
	//	 	for records: server will deal with the Duplicate events; 
	if (request->records().size() > 0) {
		if (!user_manager->set_events_for_others(request->gay(), request->records())) {
			response->set_status(false);
			response->set_error_msg("Fail to set events for others");
		} else {
			if (!db_helper->push_records(request->records())) {
				response->set_status(false);
				response->set_error_msg("Fail to push records into DB");
			} else {
				response->set_status(true);
			}
		}
	} else {
		response->set_status(true);
	}
	context->set_session_field("status", response->status() ? "true" : "false");
	if (!response->status()) {
		context->set_session_field("err_msg", response->has_error_msg() ? response->error_msg() : "unkown");
	}

	// 1. server side don't remember which events you have already get, it just return 
	// records begin from request->begin_index()
	// 2. client side deal with data integrity, which means that it will retry if the last results is suspicious
	if (request->has_begin_index()) {
		::google::protobuf::RepeatedPtrField<Record>* others_records = response->mutable_records();
		if (!user_manager->get_events_for_self(request->gay(), request->begin_index(), request->max_line(),
				db_helper, others_records)) {
			LOG(ERROR, "Fail to get events for self");
		}
	}

	return done->Run();
}

}
