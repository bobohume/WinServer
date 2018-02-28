CREATE TABLE [dbo].[Tbl_TopRank] (
	[UID]	   VARCHAR(37)  NOT NULL,
	[Type]     TINYINT      NOT NULL,
    [Val0]     INT          CONSTRAINT		[DF_Tbl_TopRank_Val0]		DEFAULT((0))		NOT NULL,
	[Val1]     INT          CONSTRAINT		[DF_Tbl_TopRank_Val1]		DEFAULT((0))		NOT NULL,
	[Val2]     INT          CONSTRAINT		[DF_Tbl_TopRank_Val2]		DEFAULT((0))		NOT NULL,
    [LastTime] DATETIME     CONSTRAINT		[DF_Tbl_TopRank_LastTime]	DEFAULT((getdate()))NOT NULL,
);













