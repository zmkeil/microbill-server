#ifndef APP_MICROBILL_BILL_SERVICE_IMPL_H
#define APP_MICROBILL_BILL_SERVICE_IMPL_H

#include "bill.pb.h"

namespace microbill {

class BillServiceImpl : public BillService {

public:
	BillServiceImpl() {}
	virtual ~BillServiceImpl() {}
	virtual void update(google::protobuf::RpcController* cntl_base,
			const BillRequest* request,
			BillResponse* response,
			google::protobuf::Closure* done);

	virtual void property(google::protobuf::RpcController* cntl_base,
			const PropertyRequest* request,
			PropertyResponse* response,
			google::protobuf::Closure* done);
};

}

#endif
