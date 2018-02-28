CREATE TABLE [dbo].[Tbl_Mount] (
    [PlayerID]       INT      NOT NULL,
    [Pos]            TINYINT  NOT NULL,
    [MountDataID]    INT      NOT NULL,
    [Level]          SMALLINT NOT NULL,
    [Duration]       INT      NOT NULL,
    [StartTime]      INT      NOT NULL,
    [SkillID0]       INT      NOT NULL,
    [SkillID1]       INT      NOT NULL,
    [SkillID2]       INT      NOT NULL,
    [Status]         TINYINT  NOT NULL,
    [LockedLeftTime] INT      NOT NULL
);

