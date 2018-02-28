CREATE TABLE [dbo].[Tbl_OrgGift] (
    [OrgId]			INT																			NOT NULL,
	[PlayerId]		INT																			NOT NULL,
	[UID]		    VARCHAR(37)																	NOT NULL,
    [GiftId]		INT			CONSTRAINT	[DF_Tbl_OrgGift_GiftId]			DEFAULT((0))		NOT NULL,
	[PlayerName]	VARCHAR(32)	CONSTRAINT	[DF_Tbl_OrgGift_PlayerName]		DEFAULT((''))		NOT NULL,	
);