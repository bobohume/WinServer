CREATE TABLE [dbo].[Tbl_Pve] (
    [PlayerID]		 INT																			NOT NULL,
    [TargetID]       INT																			NOT NULL,
	[Type]			 TINYINT																		NOT NULL,
	[Unqiue]		 INT																			NOT NULL,
	[CardNum]		 TINYINT	  CONSTRAINT	[DF_Tbl_Pve_CardNum]			DEFAULT((0))		NOT NULL,
	[Score]		     INT		  CONSTRAINT	[DF_Tbl_Pve_Score]				DEFAULT((0))		NOT NULL,
	[AddScore]		 INT		  CONSTRAINT	[DF_Tbl_Pve_AddScore]			DEFAULT((0))		NOT NULL,
	[BattleVal]		 INT		  CONSTRAINT	[DF_Tbl_Pve_BattleVal]			DEFAULT((0))		NOT NULL,
    [Time]			 DATETIME	  CONSTRAINT	[DF_Tbl_Pve_Time]				DEFAULT((getdate()))NOT NULL, 
);
