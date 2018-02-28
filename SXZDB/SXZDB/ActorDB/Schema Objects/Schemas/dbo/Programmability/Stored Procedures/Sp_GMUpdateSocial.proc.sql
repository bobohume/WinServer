CREATE PROCEDURE [dbo].[Sp_GMUpdateSocial]
@PlayerID		INT,
@FriendID		INT,
@TYPE			TINYINT,
@destTYPE	    TINYINT,
@FriendValue	INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	BEGIN TRANSACTION GMUpdateSocial
	IF EXISTS(SELECT 1 FROM TBL_SOCIAL WHERE player=@PlayerID AND Target=@FriendID)
	BEGIN
		UPDATE Tbl_Social SET [type]=@TYPE, FriendValue=@FriendValue
			WHERE player=@PlayerID AND Target=@FriendID
	END
	ELSE
	BEGIN		
		INSERT Tbl_Social (Player,Target,[type],friendvalue)
			VALUES (@PlayerID, @FriendID, @Type, @FriendValue)
	END

	IF EXISTS(SELECT 1 FROM TBL_SOCIAL WHERE player=@FriendID AND Target=@PlayerID)
	BEGIN
		UPDATE Tbl_Social SET [type]=@destTYPE, FriendValue=@FriendValue
			WHERE player=@FriendID AND Target=@PlayerID
	END
	ELSE
	BEGIN
		INSERT Tbl_Social (Player,Target,[type],friendvalue)
			VALUES (@FriendID, @PlayerID, @destTYPE, @FriendValue)
	END
	COMMIT TRANSACTION GMUpdateSocial
END