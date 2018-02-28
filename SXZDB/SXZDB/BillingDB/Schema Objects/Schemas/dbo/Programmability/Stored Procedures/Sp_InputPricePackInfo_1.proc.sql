CREATE PROCEDURE [dbo].[Sp_InputPricePackInfo]
@param VARCHAR (MAX)
AS
BEGIN
	SET NOCOUNT ON 
	SET XACT_ABORT ON
	
	SELECT @param = 'SELECT '+ REPLACE(@param,';',CHAR(13)+' UNION SELECT ')
	CREATE TABLE #
	(
		PriceTaxonID		int NOT NULL, 
		PriceRackID			int NOT NULL,
		PriceID				bigint NOT NULL,
		CreateDate			DATETIME	DEFAULT GETDATE()
	)
	DECLARE @SQL VARCHAR(MAX)
	DECLARE @ISERR INT
	SELECT @ISERR = -1
	SELECT @SQL = 'INSERT #(PriceTaxonID,PriceRackID,PriceID)'+CHAR(13)+@param
	EXEC (@SQL)
	
	BEGIN TRAN
		DELETE FROM [dbo].[Tbl_PriceTaxon]
			WHERE EXISTS(SELECT * FROM # B WHERE B.PriceID = Tbl_PriceTaxon.PriceID AND B.PriceRackID = Tbl_PriceTaxon.PriceRackID)
		INSERT [dbo].[Tbl_PriceTaxon]
			SELECT * FROM #
		UPDATE Tbl_Price_Version SET Version = Version + 1
		IF @@ROWCOUNT = 0 
			INSERT Tbl_Price_Version SELECT 1,GETDATE()
		SELECT @ISERR = 0
	COMMIT TRAN
	RETURN @ISERR

END