CREATE TABLE [dbo].[Tbl_Battle] (
    [PlayerID]       INT		  NOT NULL,
	[Card0]			 VARCHAR(37)  CONSTRAINT	[DF_Tbl_Battle_Card0]					DEFAULT((''))		NOT NULL,
	[Card1]			 VARCHAR(37)  CONSTRAINT	[DF_Tbl_Battle_Card1]					DEFAULT((''))		NOT NULL,
	[Card2]			 VARCHAR(37)  CONSTRAINT	[DF_Tbl_Battle_Card2]					DEFAULT((''))		NOT NULL,
	[Card3]			 VARCHAR(37)  CONSTRAINT	[DF_Tbl_Battle_Card3]					DEFAULT((''))		NOT NULL,
	[CardNum]		 TINYINT	  CONSTRAINT	[DF_Tbl_Battle_CardNum]					DEFAULT((0))		NOT NULL,
	[GateId]		 INT		  CONSTRAINT	[DF_Tbl_Battle_GateId]					DEFAULT((0))		NOT NULL,
);









