CREATE TABLE [dbo].[Tbl_Equip] (
    [UID]			 VARCHAR(64)																	NOT NULL,
    [PlayerID]       INT		  CONSTRAINT	[DF_Tbl_Equip_PlayerID]			DEFAULT((0))		NOT NULL,
    [ItemID]		 INT	   	  CONSTRAINT	[DF_Tbl_Equip_ItemID]			DEFAULT((0))		NOT NULL,
    [Quantity]       SMALLINT     CONSTRAINT	[DF_Tbl_Equip_Quantity]			DEFAULT((0))		NOT NULL,
);









