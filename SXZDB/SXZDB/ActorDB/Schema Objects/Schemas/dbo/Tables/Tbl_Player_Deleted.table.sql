CREATE TABLE [dbo].[Tbl_Player_Deleted] (
    [AccountID]      INT          NOT NULL,
    [PlayerID]       INT          NOT NULL,
    [PlayerName]     VARCHAR (32) NOT NULL,
    [DeletedDate]    DATETIME     NOT NULL,
    [DBVersion]      INT          NOT NULL,
    [Sex]            INT          NOT NULL,
    [Level]          INT          NOT NULL,
    [Family]         INT          NOT NULL,
    [FamilyPos]      INT          NOT NULL,
    [FirstClass]     INT          NOT NULL,
    [FaceImage]      INT          NOT NULL,
    [MasterLevel]    INT          NOT NULL,
    [Activity]       INT          NOT NULL,
    [MPMStudyLevel]  INT          NULL,
    [LastLoginTime]  DATETIME     NOT NULL,
    [LastLogoutTime] DATETIME     NOT NULL,
    [BLOBData]       IMAGE        NOT NULL
);



