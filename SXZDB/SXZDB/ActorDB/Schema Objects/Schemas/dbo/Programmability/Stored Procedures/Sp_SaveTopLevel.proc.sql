CREATE PROCEDURE [dbo].[Sp_SaveTopLevel]
@TopLevel	INT
AS
BEGIN
	SET NOCOUNT ON
	IF EXISTS(SELECT 1 FROM Tbl_World)
		UPDATE Tbl_World SET TopLevel=@TopLevel		
	ELSE
		INSERT INTO Tbl_World (TopLevel,TopDisaster) VALUES (@TopLevel,0)
END