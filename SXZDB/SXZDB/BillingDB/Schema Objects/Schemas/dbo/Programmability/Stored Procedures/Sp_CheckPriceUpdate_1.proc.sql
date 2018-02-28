CREATE PROCEDURE [dbo].[Sp_CheckPriceUpdate]

AS
BEGIN
	SET NOCOUNT ON
	DECLARE @Version INT
	DECLARE @CurrentTime DATETIME, @LastTime DATETIME
	SET @CurrentTime = GETDATE()

	SELECT @Version=Version,@LastTime=LastTime FROM Tbl_Price_Version
	IF @@ROWCOUNT = 0
	BEGIN
		INSERT INTO Tbl_Price_Version(Version,LastTime) VALUES(1, @CurrentTime)
		SET @Version = 1
	END
	ELSE
	BEGIN
		IF DATEDIFF(DAY,@LastTime,@CurrentTime) <> 0
		BEGIN
			SET @Version = @Version + 1
			UPDATE Tbl_Price_Version SET Version=@Version,LastTime=@CurrentTime
		END
	END
	SELECT @Version
END