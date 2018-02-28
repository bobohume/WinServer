CREATE TABLE [dbo].[Tbl_Boss] (
    [BossId]		INT																			NOT NULL,
	[DataId]		INT			CONSTRAINT	[DF_Tbl_Boss_DataId]			DEFAULT((0))		NOT NULL,
	[Hp]			INT			CONSTRAINT	[DF_Tbl_Boss_Hp]				DEFAULT((0))		NOT NULL,
	[Type]			INT			CONSTRAINT	[DF_Tbl_Boss_Type]				DEFAULT((0))		NOT NULL,
	[CreateTime]	DATETIME	CONSTRAINT	[DF_Tbl_Boss_CreateTime]		DEFAULT((getdate()))NOT NULL,
	[EndTime]		DATETIME	CONSTRAINT	[DF_Tbl_Boss_EndTime]			DEFAULT((getdate()))NOT NULL,
);