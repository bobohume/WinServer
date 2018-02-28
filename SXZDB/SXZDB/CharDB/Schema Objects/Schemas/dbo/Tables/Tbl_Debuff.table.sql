CREATE TABLE [dbo].[Tbl_Debuff] (
    [PlayerID] INT      NOT NULL,
    [Pos]      INT      NOT NULL,
    [BuffID]   INT      NOT NULL,
    [LeftTime] INT      NOT NULL,
    [Count]    SMALLINT NOT NULL,
    [Self]     TINYINT  NOT NULL
);



