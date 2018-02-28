CREATE PROCEDURE [dbo].[Sp_SaveTopDisaster]
@TopDisaster INT
AS
BEGIN
	SET NOCOUNT ON
	IF EXISTS (SELECT 1 FROM Tbl_World)
		UPDATE Tbl_World SET TopDisaster=@TopDisaster
	ELSE
		INSERT INTO Tbl_World (TopLevel,TopDisaster) VALUES (80,@TopDisaster)
END