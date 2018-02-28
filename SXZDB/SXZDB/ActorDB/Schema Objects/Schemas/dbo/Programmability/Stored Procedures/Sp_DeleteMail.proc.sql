-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE Sp_DeleteMail 
@recver INT,
@param VARCHAR(2000)
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON

	BEGIN TRANSACTION DeleteMail

	DECLARE @sqlstr NVARCHAR(2000) --查询字符串
	SELECT @sqlstr='INSERT INTO Tbl_MailList_Deleted SELECT *,getDate() FROM Tbl_MailList
		WHERE recver='+ CAST(@recver AS NVARCHAR(20)) +'AND id IN (' + @param + ')'
	EXEC( @sqlstr)

	SELECT @sqlstr='DELETE FROM Tbl_MailList WHERE recver='+ CAST(@recver AS NVARCHAR(20)) +'AND id IN (' + @param + ')'
	EXEC (@sqlstr)

	COMMIT TRANSACTION DeleteMail
END
