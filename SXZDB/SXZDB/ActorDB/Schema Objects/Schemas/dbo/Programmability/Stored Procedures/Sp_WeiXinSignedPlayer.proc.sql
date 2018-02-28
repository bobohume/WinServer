/*CREATE PROCEDURE [dbo].[Sp_WeiXinSignedPlayer]
@AccountName VARCHAR(100)
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @ERR INT
	DECLARE @PlayerID INT
	DECLARE @Gold	INT
	DECLARE @Sign INT
	DECLARE @SignTime DATETIME
	DECLARE @CONVERTIMES INT
	SET @ERR = 0
	SET @PlayerID = 0
	SET @Gold = 0
	SET @Sign = 0
	SET @SignTime = '1989-05-10 00:00:00.000'
	SET @CONVERTIMES = 7

	BEGIN TRANSACTION WeiXinSignedPlayer
		SELECT @PlayerID = B.PlayerID, @Gold = B.Gold, @Sign = B.Sign, @SignTime = B.SignTime FROM Tbl_Account A INNER JOIN Tbl_Player B ON A.AccountId = B.AccountID where A.AccountName = @AccountName
		IF @@ROWCOUNT <> 0
		BEGIN
			IF DATEDIFF(DAY, @SignTime, GETDATE()) >= 1
			BEGIN
				SET @Sign = @Sign + 1
				SET @SignTime = GETDATE()
				IF @Sign >= @CONVERTIMES
				BEGIN
					SET	@Gold = @Gold + 1
					Update Tbl_Player SET Gold = @Gold, Sign = 0, SignTime = GETDATE() WHERE PlayerID = @PlayerID
				END
				ELSE
				BEGIN
					Update Tbl_Player SET Sign = @Sign, SignTime = GETDATE() WHERE PlayerID = @PlayerID
				END
			END
			ELSE
				SET @ERR = 2
			END
		ELSE
		BEGIN
			SET @ERR = 1
		END

		IF @ERR <> 0
			ROLLBACK TRANSACTION WeiXinSignedPlayer
		ELSE
			COMMIT TRANSACTION WeiXinSignedPlayer
		
	SELECT @ERR AS 'Result', @PlayerID AS 'PlayerID', @Gold AS 'Card', @Sign AS 'Sign', @SignTime AS 'SignTime'
END*/