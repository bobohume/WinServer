-- =============================================
-- Author:		<LivenHotch>
-- Create date: <2010-05-27>
-- Description:	<保存社会关系数据>
-- modifyer:    <Daniel>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_SaveSocial]
@PLAYERID		INT,
@RECORD			VARCHAR(MAX)
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	BEGIN TRANSACTION SaveSocial
	DELETE FROM Tbl_Social WHERE Player=@PLAYERID
	SET @RECORD = REPLACE(@RECORD, ';#', ' UNION ALL SELECT ' + CAST(@PLAYERID AS VARCHAR(12)))
	SET @RECORD = 'INSERT INTO Tbl_Social (Player,Target,[Type],FriendValue) SELECT ' + @RECORD
	EXECUTE(@RECORD)
	COMMIT TRANSACTION SaveSocial
END




