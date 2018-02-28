CREATE TABLE [dbo].[Tbl_ScheduleBuff] (
    [PlayerID]  INT     NOT NULL,
    [BuffID]    INT     NOT NULL,
    [NeedTimes] INT     NOT NULL,
    [IsVisible] TINYINT NOT NULL,
    [IsAuto]    TINYINT NOT NULL,
    [IsHit]     TINYINT NOT NULL
);

