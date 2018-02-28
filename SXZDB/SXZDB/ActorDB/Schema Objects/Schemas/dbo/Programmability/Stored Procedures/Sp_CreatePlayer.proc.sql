/*--------------------------------------------------------------------------------------------------------------
/ 功能描述:	创建角色
/---------------------------------------------------------------------------------------------------------------*/
CREATE PROCEDURE  [dbo].[Sp_CreatePlayer]
@AccountId		INT,		--帐号ID
@PlayerId		INT,		--角色ID
@PlayerName		VARCHAR(20),--角色名称
@Sex			INT,		--角色性别
@Family			INT,		--角色门宗
@FirstClass		INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @ERR INT
	SET @ERR = -1

	BEGIN TRANSACTION CreatePlayer
	--检查角色名称
	--IF EXISTS(SELECT PlayerID FROM Tbl_Player WHERE PlayerName = @PlayerName OR PlayerID = @PlayerId)
	IF EXISTS(SELECT PlayerID FROM Tbl_Player WHERE  PlayerID = @PlayerId)
		SET @ERR = -2

	IF @ERR = -1
	BEGIN
		--检查当前帐号已有角色个数
		SELECT @ERR = CASE WHEN COUNT(PlayerID)>=1 THEN -3 ELSE -1 END FROM Tbl_Player WHERE AccountID = @AccountID
		IF @ERR = -1
		BEGIN
			--插入角色信息，其他值为默认
			INSERT INTO Tbl_Player (AccountId, PlayerID, PlayerName,Sex,[Level],Family, FirstClass,ZoneId,Gold) 
				VALUES (@AccountId, @PlayerID, @PlayerName,@Sex,0,@Family,@FirstClass,0,0)

			SET @ERR = 0
		END
	END

	IF @ERR <> 0
		ROLLBACK TRANSACTION CreatePlayer
	ELSE
		COMMIT TRANSACTION CreatePlayer

	SELECT @ERR,@PlayerID
END

