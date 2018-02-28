CREATE TABLE [dbo].[TBL_ORG] (
    [Id]         INT			 IDENTITY (1, 1)										NOT NULL,
    [OrgId]		 INT			 CONSTRAINT [DF_TBL_ORG_OrgId]		DEFAULT((0))		NOT NULL,
    [OrgName]	 VARCHAR (32)	 CONSTRAINT [DF_TBL_ORG_OrgName]	DEFAULT((''))		NOT NULL,
	[Delete]     TINYINT         CONSTRAINT [DF_TBL_ORG_Delete]		DEFAULT((0))		NOT NULL,
);











