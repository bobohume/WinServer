CREATE TABLE [dbo].[Tbl_OrgLog] (
    [OrgId]			INT																			NOT NULL,
    [CreateTime]	INT																			NOT NULL,
    [SourceName]	VARCHAR(32)	   																NOT NULL,
    [TargetName]    VARCHAR(32)	CONSTRAINT	[DF_Tbl_OrgLog_TargetName]		DEFAULT((''))		NOT NULL,
	[OptType]		INT			CONSTRAINT	[DF_Tbl_OrgLog_OptType]			DEFAULT((0))		NOT NULL,
	[Val0]			INT			CONSTRAINT	[DF_Tbl_OrgLog_Val0]			DEFAULT((0))		NOT NULL,
	[Val1]			INT			CONSTRAINT	[DF_Tbl_OrgLog_Val1]			DEFAULT((0))		NOT NULL,
);