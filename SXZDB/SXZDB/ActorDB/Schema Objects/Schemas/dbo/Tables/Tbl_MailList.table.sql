CREATE TABLE [dbo].[Tbl_MailList] (
    [id]          INT          IDENTITY (1, 1) NOT NULL,
    [sender]      INT		   CONSTRAINT		[DF_Tbl_MailList_sender]		DEFAULT((0))	 		NOT NULL,
    [sender_name] VARCHAR (32) CONSTRAINT		[DF_Tbl_MailList_sender_name]	DEFAULT((''))	 		NOT NULL,
    [money]       INT          CONSTRAINT		[DF_Tbl_MailList_money]			DEFAULT((0))	 		NOT NULL,
    [itemId0]     INT          CONSTRAINT		[DF_Tbl_MailList_itemId0]		DEFAULT((0))			NOT NULL,
	[itemId1]     INT          CONSTRAINT		[DF_Tbl_MailList_itemId1]		DEFAULT((0))			NOT NULL,
	[itemId2]     INT          CONSTRAINT		[DF_Tbl_MailList_itemId2]		DEFAULT((0))			NOT NULL,
	[itemId3]     INT          CONSTRAINT		[DF_Tbl_MailList_itemId3]		DEFAULT((0))			NOT NULL,
    [item_count0] SMALLINT     CONSTRAINT		[DF_Tbl_MailList_item_count0]	DEFAULT((0))			NOT NULL,
	[item_count1] SMALLINT     CONSTRAINT		[DF_Tbl_MailList_item_count1]	DEFAULT((0))			NOT NULL,
	[item_count2] SMALLINT     CONSTRAINT		[DF_Tbl_MailList_item_count2]	DEFAULT((0))			NOT NULL,
	[item_count3] SMALLINT     CONSTRAINT		[DF_Tbl_MailList_item_count3]	DEFAULT((0))			NOT NULL,
    [send_time]   DATETIME     CONSTRAINT		[DF_Tbl_MailList_item_send_time]DEFAULT((getdate()))	NOT NULL,
    [recver]      INT          CONSTRAINT		[DF_Tbl_MailList_recver]		DEFAULT((0))			NOT NULL,
    [recver_name] VARCHAR (32) CONSTRAINT		[DF_Tbl_MailList_recver_name]	DEFAULT((''))			NOT NULL,
    [readed]      TINYINT      CONSTRAINT		[DF_Tbl_MailList_readed]		DEFAULT((0))			NOT NULL,
    [isSystem]    TINYINT      CONSTRAINT		[DF_Tbl_MailList_isSystem]		DEFAULT((0))			NOT NULL,
    [title]       VARCHAR (32) CONSTRAINT		[DF_Tbl_MailList_title]			DEFAULT((''))			NOT NULL,
    [msg]         IMAGE        CONSTRAINT		[DF_Tbl_MailList_msg]			DEFAULT((0x00))			NOT NULL,
    [isReturn]    TINYINT      CONSTRAINT		[DF_Tbl_MailList_isReturn]		DEFAULT((0))			NOT NULL,
    [return_time] DATETIME     CONSTRAINT		[DF_Tbl_MailList_return_time]	DEFAULT((getdate()))	NOT NULL,
    [recvFlag]    TINYINT      CONSTRAINT		[DF_Tbl_MailList_recvFlag]		DEFAULT((0))			NOT NULL
);







