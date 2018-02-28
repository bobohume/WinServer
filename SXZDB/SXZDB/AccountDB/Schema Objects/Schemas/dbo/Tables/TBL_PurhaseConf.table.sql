CREATE TABLE [dbo].[TBL_PurhaseConf] (
 	[product_id] [varchar](32) NOT NULL,
	[item_id] [varchar](32) NOT NULL CONSTRAINT [DF_TBL_PurhaseConf_item_id]  DEFAULT (''),
	[Gold] [int] NOT NULL CONSTRAINT [DF_TBL_PurhaseConf_Gold]  DEFAULT ((0)),
	[Money] [int] NOT NULL CONSTRAINT [DF_TBL_PurhaseConf_Money] DEFAULT ((0)),
	[CopperCoil] [int] NOT NULL CONSTRAINT [DF_TBL_PurhaseConf_CopperCoil]  DEFAULT ((0)),
	[SilverCoil] [int] NOT NULL CONSTRAINT [DF_TBL_PurhaseConf_ilverCoil] DEFAULT ((0)),
	[GoldCoil] [int] NOT NULL CONSTRAINT [DF_TBL_PurhaseConf_GoldCoil] DEFAULT ((0)),
);

