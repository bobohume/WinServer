-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-10-26>
-- Description:	<查询邮件总量>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_LoadMailForPage] 
@Recever	INT

/*
@pagecount	INT,	--页数(注:页数从0开始)
@pagesize	INT		--每页行数
*/
AS
BEGIN
	SET NOCOUNT ON

	SELECT TOP 120 id,sender,sender_name,[money],itemId0,itemId1,itemId2,itemId3,item_count0,item_count1,item_count2,item_count3,send_time,readed,isSystem,recvFlag,title
		FROM Tbl_MailList WHERE recver=@Recever AND isSystem=0 UNION ALL 
	SELECT TOP 200 id,sender,sender_name,[money],itemId0,itemId1,itemId2,itemId3,item_count0,item_count1,item_count2,item_count3,send_time,readed,isSystem,recvFlag,title
		FROM Tbl_MailList WHERE recver=@Recever AND isSystem=1 ORDER BY id
/*
	DECLARE @sqlstr NVARCHAR(2000) --查询字符串
	DECLARE @readed INT
	SET @pagecount = @pagecount + 1 
	SELECT @sqlstr='SELECT id,sender,sender_name,[money],itemId,item_count,send_time,readed,isSystem,title'
		+CHAR(13)+'FROM Tbl_MailList WHERE recver='+ CAST(@Recever AS NVARCHAR(20)) + ' ORDER BY id DESC'
	DECLARE @P1 INT,--P1是游标的id 
	@rowcount INT 
	EXEC sp_cursoropen @P1 OUTPUT,@sqlstr,@scrollopt=1,@ccopt=1,@rowcount=@rowcount OUTPUT 
	SELECT @readed = COUNT(id) FROM Tbl_MailList WHERE recver=@Recever AND readed=0
	SELECT @rowcount as 'rowcount',CEILING(1.0*@rowcount/@pagesize) AS 'allpage',@pagecount AS 'page', @readed AS 'readed' 
	SET @pagecount=(@pagecount-1)*@pagesize+1 
	EXEC sp_cursorfetch @P1,16,@pagecount,@pagesize 
	EXEC sp_cursorclose @P1 
*/
END
