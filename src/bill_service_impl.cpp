
#include "bill_service_impl.h"

namespace microbill {

void BillServiceImpl::update(google::protobuf::RpcController* cntl_base,
			const BillRequest* request,
			BillResponse response,
			google::protobuf::Closure* done)
{
	(void) cntl_base;


	return done->Run();
}


}
