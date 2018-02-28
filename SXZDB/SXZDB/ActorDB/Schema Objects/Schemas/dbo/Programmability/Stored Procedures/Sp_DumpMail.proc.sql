CREATE PROCEDURE [dbo].[Sp_DumpMail] 
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	BEGIN TRANSACTION DumpMail
	--退还7天未取的物品邮件
	UPDATE dbo.Tbl_MailList SET sender=A.recver, sender_name=A.recver_name,
			recver=A.sender, recver_name=A.sender_name, isReturn=1, return_time=GETDATE()
		FROM (SELECT id, sender, sender_name, recver,recver_name
			FROM dbo.Tbl_MailList
			WHERE isReturn = 0 AND isSystem = 0 AND ([money]<>0 OR [itemid0]<>0) 
				AND DATEDIFF(DAY, send_time, GETDATE()) > 7) A
		WHERE Tbl_MailList.id = A.id
	
	--删除退还7天后仍未取的物品邮件到删除表
	INSERT  dbo.Tbl_MailList_Deleted
		SELECT *,GETDATE() FROM dbo.Tbl_MailList
		WHERE isReturn = 1 AND isSystem = 0 AND DATEDIFF(DAY, return_time, GETDATE()) > 7
	
	--删除退还7天后仍未取的物品邮件
	DELETE FROM dbo.Tbl_MailList
		WHERE isReturn = 1 AND isSystem = 0 AND DATEDIFF(DAY, return_time, GETDATE()) > 7

	--删除7天的文本邮件到删除表
	INSERT dbo.Tbl_MailList_Deleted
		SELECT *,GETDATE() FROM dbo.Tbl_MailList
		WHERE isSystem = 0 AND ([money]=0 AND [itemid0]=0) AND DATEDIFF(DAY, send_time, GETDATE()) > 7

	--删除7天的文本邮件
	DELETE FROM dbo.Tbl_MailList
		WHERE isSystem = 0 AND ([money]=0 AND [itemid0]=0) AND DATEDIFF(DAY, send_time, GETDATE()) > 7
	
	COMMIT TRANSACTION DumpMail
END