CREATE TABLE [dbo].[TBL_CodeHistory](
	[UID] [varchar](33) NOT NULL,
	[Order] [varchar](120) CONSTRAINT [TBL_CodeHistory_Order] DEFAULT(('')) NOT NULL,
	[Status] [int]		   CONSTRAINT [TBL_CodeHistory_Status] DEFAULT((0)) NOT NULL
)