CREATE PROCEDURE [dbo].[Sp_SaveStallShopSetting]
@MaxStalls	INT,
@TotalFlourish INT
AS
BEGIN
	SET NOCOUNT ON
	IF EXISTS(SELECT 1 FROM Tbl_World)
		UPDATE Tbl_World SET MaxStalls=@MaxStalls, TotalFlourish=@TotalFlourish
	ELSE
		INSERT INTO Tbl_World (MaxStalls,TotalFlourish) VALUES (@MaxStalls,@TotalFlourish)
END