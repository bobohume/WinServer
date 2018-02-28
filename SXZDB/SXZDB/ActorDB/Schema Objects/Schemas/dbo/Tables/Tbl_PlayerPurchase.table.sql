CREATE TABLE [dbo].[Tbl_PlayerPurchase] (
    [ID]			 INT          IDENTITY (1, 1) NOT NULL,	
    [PlayerID]       INT		  CONSTRAINT	[DF_Tbl_PlayerPurchase_PlayerID]		DEFAULT((0))		NOT NULL,		
	[Gold]			 INT		  CONSTRAINT	[DF_Tbl_PlayerPurchase_Gold]			DEFAULT((0))		NOT NULL,	
	[Price]			 FLOAT		  CONSTRAINT	[DF_Tbl_PlayerPurchase_Price]			DEFAULT((0))		NOT NULL,
    [PurchaseTime]	 DATETIME     CONSTRAINT	[DF_Tbl_PlayerPurchase_LastLoginTime]	DEFAULT(getdate())	NOT NULL,
);