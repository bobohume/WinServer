CREATE TABLE [dbo].[Tbl_LogPlayer] (
    [id]         BIGINT   IDENTITY (1, 1) NOT NULL,
    [sourceid]   INT	  CONSTRAINT [DF_Tbl_LogPlayer_sourceid]		DEFAULT((0))	NOT NULL,
    [targetid]   INT      CONSTRAINT [DF_Tbl_LogPlayer_targetid]		DEFAULT((0))	NOT NULL,
	[opttype]	 VARCHAR(64)	 CONSTRAINT [DF_Tbl_LogPlayer_opttype]	DEFAULT('')		NOT NULL,
    [logtime]    DATETIME CONSTRAINT [DF_Tbl_LogPlayer_logtime]			DEFAULT(getdate()) NOT NULL,
    [Val0]       VARCHAR(64)	 CONSTRAINT [DF_Tbl_LogPlayer_Val0]		DEFAULT('')		NOT NULL,
    [Val1]       VARCHAR(64)     CONSTRAINT [DF_Tbl_LogPlayer_Val1]		DEFAULT('')		NOT NULL,
    [Val2]       VARCHAR(64)     CONSTRAINT [DF_Tbl_LogPlayer_Val2]		DEFAULT('')		NOT NULL,
    [Val3]       VARCHAR(64)	 CONSTRAINT [DF_Tbl_LogPlayer_Val3]		DEFAULT('')		NOT NULL,
    [Val4]       VARCHAR(64)	 CONSTRAINT [DF_Tbl_LogPlayer_Val4]		DEFAULT('')		NOT NULL
);
