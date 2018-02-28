CREATE TABLE [dbo].[Tbl_PlayerBaseInfo] (
    [PlayerID]       INT		  NOT NULL,
	[Business]		 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Business]		DEFAULT((0))		NOT NULL,
	[Military]		 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Military]		DEFAULT((0))		NOT NULL,
	[Technology]	 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Technology]		DEFAULT((0))		NOT NULL,
	[Money]			 BIGINT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Money]			DEFAULT((0))		NOT NULL,
	[Ore]			 BIGINT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Ore]				DEFAULT((0))		NOT NULL,
	[Troops]		 BIGINT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Troops]			DEFAULT((0))		NOT NULL,
	[Attack]		 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Attack]			DEFAULT((0))		NOT NULL,
	[Hp]			 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Hp]				DEFAULT((0))		NOT NULL,
	[Land]			 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Land]			DEFAULT((0))		NOT NULL,		
	[BattleVal]		 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_BattleVal]		DEFAULT((0))		NOT NULL,
	[PLearnNum]		 TINYINT	  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_PLearnNum]		DEFAULT((0))		NOT NULL,
	[PPlantNum]		 TINYINT	  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_PPlantNum]		DEFAULT((0))		NOT NULL,
	[CardDevNum]	 TINYINT	  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_CardDevNum]		DEFAULT((0))		NOT NULL,
	[PveScore]		 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_PveScore]		DEFAULT((1000))		NOT NULL,
	[PveMoney]		 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_PveMoney]		DEFAULT((0))		NOT NULL,
	[CashGift]		 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_CashGift]		DEFAULT((0))		NOT NULL,
	[Conquest]		 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_Conquest]		DEFAULT((0))		NOT NULL,
	[PlayerFlag]	 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_PlayerFlag]		DEFAULT((0))		NOT NULL,
	[OrgId]			 INT		  CONSTRAINT	[DF_Tbl_PlayerBaseInfo_OrgId]			DEFAULT((0))		NOT NULL,
);









