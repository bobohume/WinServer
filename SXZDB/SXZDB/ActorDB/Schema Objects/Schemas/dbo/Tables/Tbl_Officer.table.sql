CREATE TABLE [dbo].[Tbl_Officer] (
    [OfficerID]		 TINYINT	  NOT NULL,
    [PlayerID]       INT		  NOT NULL,
    [Card]			 VARCHAR(37)  CONSTRAINT [DF_Tbl_Officer_Card]			DEFAULT((''))		NOT NULL,
	[Business]		 INT		  CONSTRAINT [DF_Tbl_Officer_Business]		DEFAULT((0))		NOT NULL,
	[Technology]	 INT		  CONSTRAINT [DF_Tbl_Officer_Technology]	DEFAULT((0))		NOT NULL,
	[Military]		 INT		  CONSTRAINT [DF_Tbl_Officer_Military]		DEFAULT((0))		NOT NULL,

);








