CREATE TABLE [dbo].[Tbl_City] (
	[CityID]			 INT																			NOT NULL,
    [PlayerID]			 INT																			NOT NULL, 
	[Level]				 TINYINT		CONSTRAINT [DF_Tbl_City_Level]		DEFAULT((0))				NOT NULL,
	[Support]			 SMALLINT		CONSTRAINT [DF_Tbl_City_Support]	DEFAULT((0))				NOT NULL,
	[DevelopFlag]		 TINYINT		CONSTRAINT [DF_Tbl_City_DevelopFlag]DEFAULT((0))				NOT NULL,
);