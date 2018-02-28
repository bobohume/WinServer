CREATE TABLE [dbo].[TBL_CodeGift](
	[Code]			varchar(32)				NOT NULL,
	[CodeType]		varchar(8)				not null,
	[Type]			varchar(8)				constraint[DF_TBL_CodeGift_Type]		default((''))				not null,
	[Name]			varchar(32)				constraint[DF_TBL_CodeGift_Name]		default((''))				not null,
	[PlayerID]		int						constraint[DF_TBL_CodeGift_PlayerID]	default((0))				not null,
	[CreateTime]	datetime				constraint[DF_TBL_CodeGift_CreateTime]  default(getdate())			not null,
)