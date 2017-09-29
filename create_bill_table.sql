create table if not exists bills(
	id varchar(100) not null,
	year integer not null,
	month integer not null,
	day integer not null,
	pay_earn integer not null default 0,
	gay varchar(100) not null,
	gay_address varchar(100),
	comments varchar(1024) not null,
	cost integer not null,
	submitter varchar(100),
	submit_time integer,
	submit_address varchar(256),
	last_modify_time integer,
	last_modify_address varchar(256),
	is_deleted integer not null default 0,
	PRIMARY KEY (id)
)ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='all bills in one table';


create table if not exists pockets(
	sid varchar(100) not null,
	year integer not null,
	month integer not null,
	-- maybe for person
	comments varchar(1024),
	money integer not null,
	is_deleted integer not null default 0,
    `last_update_time` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	PRIMARY KEY (sid)
)ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='pocket money, total';


create table if not exists assets(
	sid varchar(100) not null,
	year integer not null,
	month integer not null,
	day integer not null,
	-- 储存方，
	-- zm余额宝：0
	-- zm基金：1
	-- zm铜板街：2
	-- zm微贷：3
	-- zm苏州银行：4
	-- jxj铜板街：5
	-- jxj陆金所：6
	-- jxj苏州银行：7
	store_addr integer not null,
	-- 资金流动方式
	-- 结入：0
	-- 利息：1
	-- 支取：2
	-- 偿债：3
	-- 转移：4
	flow_type integer not null,
	money integer not null,
	-- 转移的方式，需要填写转移到哪
	store_addr_op integer,
	is_deleted integer not null default 0,
    `last_update_time` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	PRIMARY KEY (sid)
)ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='property, increment';

