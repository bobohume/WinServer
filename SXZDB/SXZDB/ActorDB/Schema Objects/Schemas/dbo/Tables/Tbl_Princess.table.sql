CREATE TABLE [dbo].[Tbl_Princess] (
	[UID]			VARCHAR(37)																		NOT NULL,
	[PlayerID]		INT																				NOT NULL,
	[PrincessID]	INT			CONSTRAINT		[DF_Tbl_Princess_PrincessID]	DEFAULT((0))		NOT NULL,
	[Support]		INT			CONSTRAINT		[DF_Tbl_Princess_Support]		DEFAULT((0))		NOT NULL,
	[SupportTime]	INT			CONSTRAINT		[DF_Tbl_Princess_SupportTime]	DEFAULT((0))		NOT NULL,
	[SupportTimes]	SMALLINT	CONSTRAINT		[DF_Tbl_Princess_SupportTimes]	DEFAULT((0))		NOT NULL,
	[Charm]			INT			CONSTRAINT		[DF_Tbl_Princess_Charm]			DEFAULT((0))		NOT NULL,
	[Rite]			INT			CONSTRAINT		[DF_Tbl_Princess_Rite]			DEFAULT((0))		NOT NULL,
	[Culture]		INT			CONSTRAINT		[DF_Tbl_Princess_Culture]		DEFAULT((0))		NOT NULL,
	[Battle]		INT			CONSTRAINT		[DF_Tbl_Princess_Battle]		DEFAULT((0))		NOT NULL,
	[PrincessPos]	INT			CONSTRAINT		[DF_Tbl_Princess_PrincessPos]	DEFAULT((0))		NOT NULL,
	[Skill0]		INT			CONSTRAINT		[DF_Tbl_Princess_SkillID0]		DEFAULT((0))		NOT NULL,
	[Skill1]		INT			CONSTRAINT		[DF_Tbl_Princess_SkillID1]		DEFAULT((0))		NOT NULL,
	[Skill2]		INT			CONSTRAINT		[DF_Tbl_Princess_SkillID2]		DEFAULT((0))		NOT NULL,
	[PrinceNum]		INT			CONSTRAINT		[DF_Tbl_Princess_PrincessNum]	DEFAULT((0))		NOT NULL,
	[PrinceAbility]	INT			CONSTRAINT		[DF_Tbl_Princess_PrinceAbility]		DEFAULT((0))		NOT NULL,
	[BattleVal]		INT			CONSTRAINT		[DF_Tbl_Princess_BattleVal]		DEFAULT((0))		NOT NULL,
);







