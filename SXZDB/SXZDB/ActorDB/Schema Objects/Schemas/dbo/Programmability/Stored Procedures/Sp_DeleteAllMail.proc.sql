-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-12-06>
-- Description:	<删除所有邮件>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_DeleteAllMail]
@recver int
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON

	BEGIN TRANSACTION DeleteAllMail

	SELECT id FROM Tbl_MailList
		WHERE recver=@recver AND ((itemid0 = 0 AND [money]=0) or recvFlag=1) AND readed = 1 

	INSERT INTO Tbl_MailList_Deleted SELECT *,getDate() FROM Tbl_MailList
		WHERE recver=@recver AND ((itemid0 = 0 AND [money]=0) or recvFlag=1) AND readed = 1 

	DELETE FROM Tbl_MailList WHERE id IN (SELECT id FROM Tbl_MailList
		WHERE recver=@recver AND ((itemid0 = 0 AND [money]=0) or recvFlag=1) AND readed = 1)

	COMMIT TRANSACTION DeleteAllMail


/*
	--特别处理，注意:因为客户端只最多显示30封邮件，执行"清空邮件"操作时，只能最多清除这30封邮件
	DELETE FROM Tbl_MailList WHERE id IN (SELECT TOP 30 id FROM Tbl_MailList
		WHERE recver=@recver AND itemid = 0 AND [money]=0 AND
			readed = 1 AND DataLength(item) < 2 ORDER BY id DESC)
*/

END
