CREATE TABLE [dbo].[Tbl_Skill] (
	[Series]			 INT																			NOT NULL,
    [PlayerID]			 INT																			NOT NULL, 
	[Level]				 TINYINT		CONSTRAINT [DF_Tbl_Skill_Level]		DEFAULT((0))				NOT NULL,
);