-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-06>
-- Description:	<确保帐号一定存在表中>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_MakeAccount]
@AccountName   	VARCHAR(100),	--帐号名称
@AccountId		INT,			--帐号ID
@Flag			INT			--成人标志
AS
BEGIN	
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @ID INT				--待检查的帐号ID
	DECLARE @Name VARCHAR(100)	--待检查的帐号名称
	DECLARE @IsAdult TINYINT	--防沉迷标志
	DECLARE @ERR INT			--返回的错误码

	SET @ERR = 0
	SET @ID = 0
	
	BEGIN TRANSACTION MakeAccountApple
	SELECT @ID=AccountId FROM Tbl_Account WHERE AccountId=@AccountId
	IF @@ROWCOUNT = 0
	BEGIN
		INSERT INTO Tbl_Account (AccountName, AccountId, IsAdult)
			VALUES (@AccountName, @AccountId, @Flag)
			SET @ID = @AccountId
	END
	ELSE
	BEGIN
		SET @ID = @AccountId
		IF @IsAdult <> @Flag
		BEGIN
			UPDATE Tbl_Account SET IsAdult=@Flag
				WHERE AccountId=@AccountId
		END	
	END

	COMMIT TRANSACTION MakeAccountApple
	SELECT @ERR, @ID
END
	
	