CREATE TABLE [dbo].[Tbl_PriceTaxon] (
    [PriceTaxonID] INT      NOT NULL,
    [PriceRackID]  INT      NOT NULL,
    [PriceID]      BIGINT   NOT NULL,
    [CreateDate]   DATETIME DEFAULT (getdate()) NULL
);

