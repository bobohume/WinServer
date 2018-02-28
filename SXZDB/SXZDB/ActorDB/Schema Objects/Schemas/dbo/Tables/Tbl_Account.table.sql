CREATE TABLE [dbo].[Tbl_Account] (
    [AccountId]         INT           NOT NULL,
    [AccountName]       VARCHAR (100),
    [IsAdult]           TINYINT       NOT NULL,
    [Mode]              TINYINT       NOT NULL,
    [Status]            INT           NOT NULL,
    [GMFlag]            TINYINT       NOT NULL,
    [LoginStatus]       TINYINT       NOT NULL,
    [LoginTime]         DATETIME      NOT NULL,
    [LogoutTime]        DATETIME      NOT NULL,
    [LoginIP]           VARCHAR (20)  NOT NULL,
    [LoginError]        INT           NOT NULL,
    [TotalOnlineTime]   INT           NOT NULL,
    [AccountFunction]   INT           NOT NULL,
    [dTotalOnlineTime]  INT           NOT NULL,
    [dTotalOfflineTime] INT           NOT NULL,
    [LoginLevel]        INT           NOT NULL
);







