CREATE TABLE [dbo].[Tbl_OrgApply] (
    [PlayerId]		INT																			NOT NULL,
    [OrgId]			INT																			NOT NULL,
	[PlayerName]	VARCHAR(32)	CONSTRAINT	[DF_Tbl_OrgApply_PlayerName]	DEFAULT((''))		NOT NULL,
	[BattleVal]		INT			CONSTRAINT	[DF_Tbl_OrgApply_BattleVal]		DEFAULT((0))		NOT NULL,
	[ApplyTime]		DATETIME	CONSTRAINT	[DF_Tbl_OrgApply_ApplyTime]		DEFAULT((getdate()))NOT NULL,
	[Vip]			INT			CONSTRAINT	[DF_Tbl_OrgApply_Vip]			DEFAULT((0))		NOT NULL,
	[PlayerFace]	INT			CONSTRAINT	[DF_Tbl_OrgApply_PlayerFace]	DEFAULT((0))		NOT NULL,
	[OnlineFlag]	TINYINT		CONSTRAINT	[DF_Tbl_OrgApply_OnlineFlag]	DEFAULT((0))		NOT NULL,
);