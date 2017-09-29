#ifndef APP_MICROBILL_DB_CLIENT_H
#define APP_MICROBILL_DB_CLIENT_H

#include "msg_adaptor.h"

namespace microbill {

class DBClient {

public:
	DBClient() {}
	virtual ~DBClient() {}
	virtual bool query(const SQL& sql, RecordLines* record_lines) = 0;
};

}

#endif //APP_MICROBILL_DB_CLIENT_H
