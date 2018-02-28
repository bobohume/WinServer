CREATE PROCEDURE [dbo].[Sp_CheckAccountExist]
@AccountId INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @ERR INT
	DECLARE @AccountName VARCHAR(100)
	SET @ERR = 0

	BEGIN TRANSACTION CheckAccountExist
		SELECT @AccountName = AccountName FROM Tbl_Account where AccountId = @AccountId
		IF @AccountName IS NULL
			SET @ERR = 0
		ELSE IF @AccountName = ''
			SET @ERR = 2
		ELSE
			SET @ERR = 1


		IF @ERR <> 0
			COMMIT TRANSACTION CheckAccountExist
		ELSE
			ROLLBACK TRANSACTION CheckAccountExist
		
	SELECT @ERR
END