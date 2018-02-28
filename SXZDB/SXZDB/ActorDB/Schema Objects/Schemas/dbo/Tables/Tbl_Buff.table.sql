CREATE TABLE [dbo].[Tbl_Buff] (
    [PlayerID]       INT																			NOT NULL,
    [buffId]		 INT																			NOT NULL,
    [count]          SMALLINT     CONSTRAINT	[DF_Tbl_Buff_count]				DEFAULT((0))		NOT NULL,
    [effTimes]       INT          CONSTRAINT	[DF_Tbl_Buff_effTimes]			DEFAULT((0))		NOT NULL
);









