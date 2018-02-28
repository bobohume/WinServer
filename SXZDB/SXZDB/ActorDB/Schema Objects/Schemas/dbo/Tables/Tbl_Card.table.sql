CREATE TABLE [dbo].[Tbl_Card] (
    [UID]			 VARCHAR(37)																	NOT NULL,
    [PlayerID]       INT																			NOT NULL,
    [CardID]		 INT	   	  CONSTRAINT	[DF_Tbl_Card_CardID]			DEFAULT((0))		NOT NULL,
    [Level]			 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Level]				DEFAULT((0))		NOT NULL,
	[Achievement]	 INT		  CONSTRAINT	[DF_Tbl_Card_Achievement]		DEFAULT((0))		NOT NULL,
	[OfficialRank]	 TINYINT	  CONSTRAINT	[DF_Tbl_Card_OfficialRank]		DEFAULT((0))		NOT NULL,
	[OfficialPos]	 TINYINT	  CONSTRAINT	[DF_Tbl_Card_OfficialPos]		DEFAULT((0))		NOT NULL,
	[CardMask]		 INT		  CONSTRAINT	[DF_Tbl_Card_CardMask]			DEFAULT((0))		NOT NULL,

	[Agi]			 INT		  CONSTRAINT	[DF_Tbl_Card_Agi]				DEFAULT((0))		NOT NULL,
	[Int]			 INT		  CONSTRAINT	[DF_Tbl_Card_Int]				DEFAULT((0))		NOT NULL,
	[Str]			 INT		  CONSTRAINT	[DF_Tbl_Card_Str]				DEFAULT((0))		NOT NULL,
	
	[Agi_Grow]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Agi_Grow]			DEFAULT((0))		NOT NULL, 
	[Int_Grow]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Int_Grow]			DEFAULT((0))		NOT NULL, 
	[Str_Grow]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Str_Grow]			DEFAULT((0))		NOT NULL, 	

	[Equip0]		 VARCHAR(37)  CONSTRAINT	[DF_Tbl_Equip0]					DEFAULT((''))		NOT NULL,
	[Equip1]		 VARCHAR(37)  CONSTRAINT	[DF_Tbl_Equip1]					DEFAULT((''))		NOT NULL,
	[Equip2]		 VARCHAR(37)  CONSTRAINT	[DF_Tbl_Equip2]					DEFAULT((''))		NOT NULL,

	[Skill0]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill0]			DEFAULT((0))		NOT NULL,
	[Skill1]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill1]			DEFAULT((0))		NOT NULL,
	[Skill2]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill2]			DEFAULT((0))		NOT NULL,
	[Skill3]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill3]			DEFAULT((0))		NOT NULL,
	[Skill4]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill4]			DEFAULT((0))		NOT NULL,
	[Skill5]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill5]			DEFAULT((0))		NOT NULL,
	[Skill6]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill6]			DEFAULT((0))		NOT NULL,
	[Skill7]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill7]			DEFAULT((0))		NOT NULL,
	[Skill8]		 INT		  CONSTRAINT	[DF_Tbl_Card_Skill8]			DEFAULT((0))		NOT NULL,

	[Agi_Add0]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Agi_Add0]			DEFAULT((0))		NOT NULL, 
	[Agi_Add1]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Agi_Add1]			DEFAULT((0))		NOT NULL,
	[Agi_Add2]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Agi_Add2]			DEFAULT((0))		NOT NULL, 
	[Agi_Add3]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Agi_Add3]			DEFAULT((0))		NOT NULL,
	[Int_Add0]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Int_Add0]			DEFAULT((0))		NOT NULL, 
	[Int_Add1]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Int_Add1]			DEFAULT((0))		NOT NULL,
	[Int_Add2]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Int_Add2]			DEFAULT((0))		NOT NULL, 
	[Int_Add3]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Int_Add3]			DEFAULT((0))		NOT NULL,
	[Str_Add0]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Str_Add0]			DEFAULT((0))		NOT NULL, 
	[Str_Add1]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Str_Add1]			DEFAULT((0))		NOT NULL,
	[Str_Add2]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Str_Add2]			DEFAULT((0))		NOT NULL, 
	[Str_Add3]		 SMALLINT	  CONSTRAINT	[DF_Tbl_Card_Str_Add3]			DEFAULT((0))		NOT NULL,
	[BattleVal]		 INT		  CONSTRAINT	[DF_Tbl_Card_BattleVal]			DEFAULT((0))		NOT NULL, 
    [SkillsFailedNum]BIGINT		  CONSTRAINT	[DF_Tbl_Card_SkillsFailedNum]	DEFAULT ((0))		NOT NULL, 
    [AptFailedNum]	 INT		  CONSTRAINT	[DF_Tbl_Card_AptFailedNum]		DEFAULT ((0))		NOT NULL,
);









