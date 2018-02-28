CREATE TABLE [dbo].[Tbl_BossRank] (
    [BossId]		INT																			NOT NULL,
	[PlayerId]		INT																			NOT NULL,
	[Hp]			INT			CONSTRAINT	[DF_Tbl_BossRank_Hp]			DEFAULT((0))		NOT NULL,
);