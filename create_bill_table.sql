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
	-- ���淽��
	-- zm����0
	-- zm����1
	-- zmͭ��֣�2
	-- zm΢����3
	-- zm�������У�4
	-- jxjͭ��֣�5
	-- jxj½������6
	-- jxj�������У�7
	store_addr integer not null,
	-- �ʽ�������ʽ
	-- ���룺0
	-- ��Ϣ��1
	-- ֧ȡ��2
	-- ��ծ��3
	-- ת�ƣ�4
	flow_type integer not null,
	money integer not null,
	-- ת�Ƶķ�ʽ����Ҫ��дת�Ƶ���
	store_addr_op integer,
	is_deleted integer not null default 0,
    `last_update_time` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	PRIMARY KEY (sid)
)ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COLLATE=utf8_bin COMMENT='property, increment';

