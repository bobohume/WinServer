/*CREATE PROCEDURE [dbo].[Sp_WeiXinGetPlayerInfo]
@AccountName VARCHAR(100)
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @ERR INT
	DECLARE @PlayerID INT
	DECLARE @Gold	INT
	DECLARE @Money INT
	DECLARE @Tel  VARCHAR(32)
	DECLARE @Vip INT
	SET @ERR = 0
	SET @PlayerID = 0
	SET @Gold = 0
	SET @Money = 0
	SET @Tel = ''
	SET @Vip = 0

	BEGIN TRANSACTION WeiXinGetPlayerInfo
		SELECT @PlayerID = B.PlayerID, @Gold = B.Gold, @Money = B.Money, @Tel = B.Tel, @Vip = B.VipLv FROM Tbl_Account A INNER JOIN Tbl_Player B ON A.AccountId = B.AccountID where A.AccountName = @AccountName
		IF @@ROWCOUNT <> 0
			SET @ERR = 0
		ELSE
			SET @ERR = 1

		IF @ERR <> 0
			ROLLBACK TRANSACTION WeiXinGetPlayerInfo
		ELSE
			COMMIT TRANSACTION WeiXinGetPlayerInfo
		
	SELECT @ERR AS 'Result', @PlayerID AS 'PlayerID', @Gold AS 'Card', @Money AS 'Money', @Tel AS 'Tel', @Vip AS 'Vip'
END*/