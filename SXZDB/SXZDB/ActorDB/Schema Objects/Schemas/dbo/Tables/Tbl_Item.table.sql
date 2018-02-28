CREATE TABLE [dbo].[Tbl_Item] (
    [UID]			 VARCHAR(37)																	NOT NULL,
    [PlayerID]       INT																			NOT NULL,
    [ItemID]		 INT	   	  CONSTRAINT	[DF_Tbl_Item_ItemID]			DEFAULT((0))		NOT NULL,
    [Quantity]       SMALLINT     CONSTRAINT	[DF_Tbl_Item_Quantity]			DEFAULT((0))		NOT NULL,
	[Level]			 TINYINT	  CONSTRAINT	[DF_Tbl_Item_Level]				DEFAULT((0))		NOT NULL,
	[Exp]			 INT		  CONSTRAINT	[DF_Tbl_Item_Exp]				DEFAULT((0))		NOT NULL,
);









