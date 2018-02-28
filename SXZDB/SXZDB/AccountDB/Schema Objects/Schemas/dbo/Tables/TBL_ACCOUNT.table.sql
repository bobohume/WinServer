CREATE TABLE [dbo].[TBL_ACCOUNT] (
    [ACCOUNTID]    INT            IDENTITY (1, 1) NOT NULL,
    [ACCOUNT]      VARCHAR (50),
    [PASSWORD]     VARCHAR (32),
    [REGISTERDATE] DATETIME       NOT NULL,
    [STATUS]       INT            NOT NULL,
    [VIPLV]        SMALLINT       NULL,
	[LAST_SERVER]  INT			  NULL,
);







