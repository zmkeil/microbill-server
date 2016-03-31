create table bill_test(
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
