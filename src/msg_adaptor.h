
#ifndef APP_MICROBILL_MSG_ADAPTOR_H
#define APP_MICROBILL_MSG_ADAPTOR_H

#include <vector>
#include <map>
#include <string>

namespace microbill {

typedef std::vector<std::string> EventLine;
typedef std::vector<EventLine> EventLines;

enum class SQLType {
	INSERT,
	UPDATE,
	SELECT
};
typedef std::pair<SQLType, std::string> SQL;
typedef std::vector<SQL> SQLs;

typedef std::map<std::string, std::string> RecordLine;
typedef std::vector<RecordLine> RecordLines;


// Only adaptor, store nothing
class MsgAdaptor {

public:
	MsgAdaptor() {}
	virtual ~MsgAdaptor() {}

	// for push
	virtual void push_sqls(EventLines*, SQLs*) = 0;

	// for pull
	virtual void pull_sqls(const EventLines&, SQLs*) = 0;
	virtual void set_pull_records(const EventLines&, const RecordLines&) = 0;

    // for log
    virtual std::string push_ids_str() = 0;
    virtual std::string pull_ids_str() = 0;
};

}

#endif //APP_MICROBILL_MSG_ADAPTOR_H
