CREATE TABLE [dbo].[Tbl_TimeSet] (
	[ID]			SMALLINT																			NOT NULL,
	[PlayerID]		INT																					NOT NULL,
	[Flag]			VARCHAR(37)			CONSTRAINT		[DF_Tbl_TimeSet_Flag]			DEFAULT((''))	NOT NULL,
	[Flag1]			INT					CONSTRAINT		[DF_Tbl_TimeSet_Flag1]			DEFAULT((0))	NOT NULL,
	[ExpireTime]	INT					CONSTRAINT		[DF_Tbl_TimeSet_ExpireTime]		DEFAULT((0))	NOT NULL,
);







