CREATE TABLE [dbo].[Tbl_MailList_Deleted] (
    [id]          INT          NOT NULL,
	[sender]      INT		   CONSTRAINT		[DF_Tbl_MailList_Deleted_sender]		DEFAULT((0))	 		NOT NULL,
    [sender_name] VARCHAR (32) CONSTRAINT		[DF_Tbl_MailList_Deleted_sender_name]	DEFAULT((''))	 		NOT NULL,
    [money]       INT          CONSTRAINT		[DF_Tbl_MailList_Deleted_money]			DEFAULT((0))	 		NOT NULL,
    [itemId0]     INT          CONSTRAINT		[DF_Tbl_MailList_Deleted_itemId0]		DEFAULT((0))			NOT NULL,
	[itemId1]     INT          CONSTRAINT		[DF_Tbl_MailList_Deleted_itemId1]		DEFAULT((0))			NOT NULL,
	[itemId2]     INT          CONSTRAINT		[DF_Tbl_MailList_Deleted_itemId2]		DEFAULT((0))			NOT NULL,
	[itemId3]     INT          CONSTRAINT		[DF_Tbl_MailList_Deleted_itemId3]		DEFAULT((0))			NOT NULL,
    [item_count0] SMALLINT     CONSTRAINT		[DF_Tbl_MailList_Deleted_item_count0]	DEFAULT((0))			NOT NULL,
	[item_count1] SMALLINT     CONSTRAINT		[DF_Tbl_MailList_Deleted_item_count1]	DEFAULT((0))			NOT NULL,
	[item_count2] SMALLINT     CONSTRAINT		[DF_Tbl_MailList_Deleted_item_count2]	DEFAULT((0))			NOT NULL,
	[item_count3] SMALLINT     CONSTRAINT		[DF_Tbl_MailList_Deleted_item_count3]	DEFAULT((0))			NOT NULL,
    [send_time]   DATETIME     CONSTRAINT		[DF_Tbl_MailList_Deleted_item_send_time]DEFAULT((getdate()))	NOT NULL,
    [recver]      INT          CONSTRAINT		[DF_Tbl_MailList_Deleted_recver]		DEFAULT((0))			NOT NULL,
    [recver_name] VARCHAR (32) CONSTRAINT		[DF_Tbl_MailList_Deleted_recver_name]	DEFAULT((''))			NOT NULL,
    [readed]      TINYINT      CONSTRAINT		[DF_Tbl_MailList_Deleted_readed]		DEFAULT((0))			NOT NULL,
    [isSystem]    TINYINT      CONSTRAINT		[DF_Tbl_MailList_Deleted_isSystem]		DEFAULT((0))			NOT NULL,
    [title]       VARCHAR (32) CONSTRAINT		[DF_Tbl_MailList_Deleted_title]			DEFAULT((''))			NOT NULL,
    [msg]         IMAGE        CONSTRAINT		[DF_Tbl_MailList_Deleted_msg]			DEFAULT((0x00))			NOT NULL,
    [isReturn]    TINYINT      CONSTRAINT		[DF_Tbl_MailList_Deleted_isReturn]		DEFAULT((0))			NOT NULL,
    [return_time] DATETIME     CONSTRAINT		[DF_Tbl_MailList_Deleted_return_time]	DEFAULT((getdate()))	NOT NULL,
    [recvFlag]    TINYINT      CONSTRAINT		[DF_Tbl_MailList_Deleted_recvFlag]		DEFAULT((0))			NOT NULL,
    [deletetime]  DATETIME     CONSTRAINT		[DF_Tbl_MailList_Deleted_deletetime]	DEFAULT((getdate()))			NOT NULL
);







