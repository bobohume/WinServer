CREATE TABLE [dbo].[Tbl_TimerItem] (
    [PlayerID]     INT     NOT NULL,
    [Pos]          TINYINT NOT NULL,
    [LeftTime]     INT     NOT NULL,
    [IntervalTime] INT     NOT NULL,
    [CanCancel]    TINYINT NOT NULL,
    [State]        INT     NOT NULL,
    [TimeFlag]     TINYINT NOT NULL,
    [ID]           INT     NOT NULL,
    [ItemID0]      INT     NOT NULL,
    [ItemID1]      INT     NOT NULL,
    [ItemID2]      INT     NOT NULL,
    [ItemID3]      INT     NOT NULL
);

