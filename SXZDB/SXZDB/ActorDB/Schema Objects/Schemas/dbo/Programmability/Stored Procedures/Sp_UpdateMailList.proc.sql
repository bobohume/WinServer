-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-09>
-- Description:	<更新邮件数据>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdateMailList]
@mailid			INT,
@sender			INT,
@sender_name	VARCHAR(32),
@money			INT,
@itemid0		INT,
@itemid1		INT,
@itemid2		INT,
@itemid3		INT,
@item_count0	SMALLINT,
@item_count1	SMALLINT,
@item_count2	SMALLINT,
@item_count3	SMALLINT,
@send_time		DATETIME,
@recver			INT,
@recver_name	VARCHAR(32),
@isSystem		TINYINT,
@title			VARCHAR(128),
@Message		VARCHAR(2048) = ''
AS
BEGIN
	SET NOCOUNT ON
	DECLARE @ID INT, @COUNT INT
	DECLARE @ERR INT
	SET @ERR = 0	
	
	--检查收件人ID是否存在
	IF @recver <> 0
	BEGIN
		SELECT @recver_name=PlayerName FROM Tbl_Player WHERE PlayerID=@recver
		IF @@ROWCOUNT = 0
			SET @ERR = 1	    --收件人ID不存在
	END
	ELSE
	BEGIN
		SELECT @recver=PlayerID FROM Tbl_Player WHERE PlayerName=@recver_name
		IF @@ROWCOUNT = 0
			SET @ERR = 2		--收件人名称不存在
	END

	IF @ERR = 0
	BEGIN
		IF @isSystem=0  --非系统邮件
		BEGIN
			IF @money<>0 OR @itemid0<>0		
			BEGIN
				SELECT @COUNT=COUNT(recver) FROM Tbl_MailList WHERE recver=@recver AND isSystem=0 AND ([money]<>0 OR itemid0<>0)
				IF @COUNT >= 30
					SET @ERR = 3		--带物品邮件数量超限
			END
			ELSE
			BEGIN
				SELECT @COUNT=COUNT(recver) FROM Tbl_MailList WHERE recver=@recver AND isSystem=0 AND [money]=0 AND itemid0=0
				IF @COUNT >= 90
					SET @ERR = 3		--文本邮件数量超限
			END
		END
	END

	IF @ERR = 0
	BEGIN
		IF EXISTS(SELECT 1 FROM Tbl_MailList WHERE id=@mailid)
		BEGIN
			UPDATE Tbl_MailList SET 
				[money] = @money , 
				item_count0 = @item_count0, 
				item_count1 = @item_count1, 
				item_count2 = @item_count2, 
				item_count3 = @item_count3, 
				title = @title ,
				itemId0 = @itemid0,
				itemId1 = @itemid1,
				itemId2 = @itemid2,
				itemId3 = @itemid3,
				msg	= @message
			WHERE id = @mailid
		END
		ELSE
		BEGIN
			INSERT INTO Tbl_MailList (sender,sender_name,[money],itemId0,itemId1,itemId2,itemId3,
				item_count0,item_count1,item_count2,item_count3,send_time,recver,recver_name,isSystem,title,msg)
				VALUES(@Sender,@Sender_name,@money,@itemid0,@itemid1,@itemid2,@itemid3,
				@item_count0,@item_count1,@item_count2,@item_count3,@send_time,@recver,@recver_name,@isSystem,@title,@message)
			SET @mailid = @@IDENTITY
		END		
	END

	SELECT @ERR, @mailid, @recver
END
