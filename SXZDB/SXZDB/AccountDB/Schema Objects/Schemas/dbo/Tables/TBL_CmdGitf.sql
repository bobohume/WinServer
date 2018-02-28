CREATE TABLE [dbo].[TBL_CmdGift](
	[Code]			varchar(32)				not null,
	[PlayerID]		int						not null,
	[Type]			varchar(8)				constraint[DF_TBL_CmdGift_Type]		   default((''))				not null,
	[CreateTime]	datetime				constraint[DF_TBL_CmdGift_CreateTime]  default(getdate())			not null,
)