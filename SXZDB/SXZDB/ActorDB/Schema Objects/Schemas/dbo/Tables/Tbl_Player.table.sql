CREATE TABLE [dbo].[Tbl_Player] (
    [AccountID]      INT		  CONSTRAINT	[DF_Tbl_Player_AccountID]		DEFAULT((0))		NOT NULL,
    [PlayerID]       INT																			NOT NULL,
    [PlayerName]     VARCHAR (32) CONSTRAINT	[DF_Tbl_Player_PlayerName]		DEFAULT('')			NOT NULL,
    [Sex]            TINYINT      CONSTRAINT	[DF_Tbl_Player_Sex]				DEFAULT((0))		NOT NULL,
    [Level]          INT          CONSTRAINT	[DF_Tbl_Player_Level]			DEFAULT((0))		NOT NULL,
    [Family]         INT		  CONSTRAINT	[DF_Tbl_Player_Family]			DEFAULT((0))		NOT NULL,
    [FirstClass]     INT		  CONSTRAINT	[DF_Tbl_Player_FirstClass]		DEFAULT((0))		NOT NULL,
	[ZoneId]		 INT		  CONSTRAINT	[DF_Tbl_Player_ZoneId]			DEFAULT((0))		NOT NULL,
	[Gold]			 INT		  CONSTRAINT	[DF_Tbl_Player_Gold]			DEFAULT((0))		NOT NULL,
	[DrawGold]		 INT		  CONSTRAINT	[DF_Tbl_Player_DrawGold]		DEFAULT((0))		NOT NULL,
	[VipLv]			 INT		  CONSTRAINT	[DF_Tbl_Player_VipLv]			DEFAULT((0))		NOT NULL,
    [LastLoginTime]  DATETIME     CONSTRAINT	[DF_Tbl_Player_LastLoginTime]	DEFAULT(getdate())	NOT NULL,
    [LastLogoutTime] DATETIME     CONSTRAINT	[DF_Tbl_Player_LastLogoutTime]	DEFAULT(getdate())	NOT NULL,
    [LastUpdateTime] DATETIME     CONSTRAINT	[DF_Tbl_Player_LastUpdateTime]	DEFAULT(getdate())	NOT NULL,
    [DeleteTime]     DATETIME     CONSTRAINT	[DF_Tbl_Player_DeleteTime]		DEFAULT(getdate())	NOT NULL,
);









