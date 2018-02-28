-- =============================================
-- Author:		<LivenHotch>
-- Create date: <2010-05-27>
-- Description:	<批量更新社会关系>
-- modifyer:    <Daniel>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdateSocialFriendValue]
@RECORD			VARCHAR(MAX)
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @STR VARCHAR(100), @STR1 VARCHAR(100)
	DECLARE @IDX INT, @IDX1 INT, @I INT
	DECLARE @PLAYERID VARCHAR(20), @FRIENDID VARCHAR(20), @FRIENDVALUE VARCHAR(20)
	DECLARE @SQL VARCHAR(1024),@ESQL0 VARCHAR(1024), @ESQL1 VARCHAR(1024)
	BEGIN TRANSACTION UpdateSocialFriendValue
	WHILE LEN(@RECORD) >0
	BEGIN
		SET @IDX = CHARINDEX(';',@RECORD)
		IF @IDX = 0
		BEGIN
			SET @STR = @RECORD
			SET @RECORD = ''
		END
		ELSE
		BEGIN
			SET @STR = LEFT(@RECORD, @IDX -1)
			SET @RECORD = RIGHT(@RECORD, LEN(@RECORD) - @IDX)
		END
		
		IF @STR <> ''
		BEGIN
			SET @SQL = 'UPDATE TBL_SOCIAL SET FriendValue='
			SET @I = 0
			WHILE LEN(@STR) > 0
			BEGIN
				SET @IDX1 = CHARINDEX(',', @STR)
				IF @IDX1 = 0
				BEGIN
					SET @STR1 = @STR
					SET @STR = ''
				END
				ELSE
				BEGIN
					SET @STR1 = LEFT(@STR, @IDX1 -1)
					SET @STR = RIGHT(@STR, LEN(@STR) - @IDX1)
				END

				IF @I = 0
					SET @PLAYERID = @STR1
				ELSE IF @I = 1
					SET @FRIENDID = @STR1
				ELSE
					SET @FRIENDVALUE = @STR1

				SET @I = @I + 1
			END

			SET @ESQL0 = @SQL + @FRIENDVALUE + ' WHERE PLAYER=' + @PLAYERID + ' AND TARGET=' + @FRIENDID
			EXECUTE(@ESQL0)
			SET @ESQL1 = @SQL + @FRIENDVALUE + ' WHERE PLAYER=' + @FRIENDID + ' AND TARGET=' + @PLAYERID
			EXECUTE(@ESQL1)
		END
	END
	COMMIT TRANSACTION UpdateSocialFriendValue
END




