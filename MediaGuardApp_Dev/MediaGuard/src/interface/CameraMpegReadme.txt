CREATE TABLE `ft_camera_mpeg` (
	`id` BIGINT(20) NOT NULL DEFAULT '0',
	`device_id` INT(11) NOT NULL,
	`camera_Id` INT(11) NOT NULL,
	`mpeg_filename` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'utf8mb3_general_ci',
	`group_timestamp` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'default value = 0 ; for merge by group，fill in the id(Merge GroupId),like batch product GroupId=1601018246',
	`file_size` BIGINT(20) NOT NULL DEFAULT '0',
	`file_fomat` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'utf8mb3_general_ci',
	`is_group` BIT(1) NOT NULL DEFAULT b'0',
	`start_timestamp` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'e.g. , start_timestamp=1601018000 this value should come from generate_filename when begin to write',
	`end_timestamp` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'e.g. , end_timestamp=1601019000 this value fron streamHandle::do_decode while write mp4 trailer to create new value',
	`visible` TINYINT(4) NOT NULL DEFAULT '0',
	`is_format_virified` TINYINT(4) NOT NULL DEFAULT '0' COMMENT 'signed that Verification of the mpeg format ',
	`is_upload` TINYINT(4) NOT NULL DEFAULT '0' COMMENT 'is_upload of ft_camera_mpeg,Uoload to cloud from device,isUploaded=1',
	`create_time` DATETIME NOT NULL DEFAULT current_timestamp(),
	PRIMARY KEY (`id`, `device_id`) USING BTREE,
	INDEX `index_mpeg_filename` (`mpeg_filename`) USING BTREE
)
COLLATE='utf8mb3_general_ci'
ENGINE=InnoDB
;




	`id` BIGINT(20) NOT NULL DEFAULT '0',
	`device_id` INT(11) NOT NULL,
	`camera_Id` INT(11) NOT NULL,
	`mpeg_filename` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'utf8mb3_general_ci',
	`group_timestamp` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'default value = 0 ; for merge by group，fill in the id(Merge GroupId),like batch product GroupId=1601018246',
	`file_size` BIGINT(20) NOT NULL DEFAULT '0',
	`file_fomat` VARCHAR(128) NOT NULL DEFAULT '' COLLATE 'utf8mb3_general_ci',
	`is_group` BIT(1) NOT NULL DEFAULT b'0',
	`start_timestamp` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'e.g. , start_timestamp=1601018000 this value should come from generate_filename when begin to write',
	`end_timestamp` BIGINT(20) NOT NULL DEFAULT '0' COMMENT 'e.g. , end_timestamp=1601019000 this value fron streamHandle::do_decode while write mp4 trailer to create new value',
	`visible` TINYINT(4) NOT NULL DEFAULT '0',
	`is_format_virified` TINYINT(4) NOT NULL DEFAULT '0' COMMENT 'signed that Verification of the mpeg format ',
	`is_upload` TINYINT(4) NOT NULL DEFAULT '0' COMMENT 'is_upload of ft_camera_mpeg,Uoload to cloud from device,isUploaded=1',
	`create_time` DATETIME NOT NULL DEFAULT current_timestamp(),