﻿ALTER TABLE [dbo].[Tbl_PriceRack]
    ADD CONSTRAINT [PK_Tbl_PriceRack] PRIMARY KEY CLUSTERED ([PriceRackID] ASC) WITH (ALLOW_PAGE_LOCKS = ON, ALLOW_ROW_LOCKS = ON, PAD_INDEX = OFF, IGNORE_DUP_KEY = OFF, STATISTICS_NORECOMPUTE = OFF);
