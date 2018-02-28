CREATE TABLE [dbo].[Tbl_OrgSkill] (
    [SkillId]		INT																			    NOT NULL,
    [OrgId]			INT																				NOT NULL,
    [Level]			SMALLINT	CONSTRAINT	[DF_Tbl_OrgSkill_Level]				DEFAULT((0))		NOT NULL,
	[Exp]			INT			CONSTRAINT	[DF_Tbl_OrgSkill_Exp]				DEFAULT((0))		NOT NULL,
);