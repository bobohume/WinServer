/*CREATE PROCEDURE SP_SendItem
    @AccountID INT,
    @PlayerID  INT,
    @SumPrice  INT
AS
BEGIN
    DECLARE @SendItemCount INT,@SendMailCount INT
    DECLARE @sender INT, 
           @sender_name VARCHAR (32), 
           @money INT, @itemid INT, 
           @item_count SMALLINT, 
           @send_time DATETIME, 
           @recver INT, 
           @recver_name VARCHAR (32), 
           @isSystem TINYINT, 
           @title VARCHAR (128), 
           @Message VARCHAR (2048)

    SELECT @SendItemCount = 0,@item_count = 0,@SendMailCount = 0

    SELECT @SendItemCount = SendItemCount
           ,@SendMailCount = SendMailCount
           ,@SumPrice = @SumPrice + SumPrice 
           FROM Tbl_SendItemList
              WHERE AccountID = @AccountID AND PlayerID = @PlayerID

    SELECT @item_count = (@SumPrice - @SendItemCount * 500)/500

    IF @item_count > 0
    BEGIN
       SELECT @sender = 0, @sender_name = '系统' ,@money = 0
              ,@itemID = 105102656 , @send_time = GETDATE() , @recver = @PlayerID
              ,@isSystem = 1 , @title = '神临天下大反馈' 
              ,@Message = '亲爱的仙友，为了庆祝神临天下正式上线同时感谢您的大力支持，附件为您购买道具的绑定元宝返利，请注意查收'
       SELECT @recver_name = PlayerName FROM SXZ_ActorDB..Tbl_Player WHERE PlayerID=@recver
       INSERT INTO SXZ_ActorDB..Tbl_MailList (sender,sender_name,[money],itemId,
              item_count,send_time,recver,recver_name,isSystem,title,msg)
              VALUES(@Sender,@Sender_name,@money,@itemID,@item_count,
              @send_time,@recver,@recver_name,@isSystem,@title,@message)
    END

    SELECT @SendItemCount = @SendItemCount + @item_count
           ,@SendMailCount = @SendMailCount + CASE WHEN @item_count > 0 THEN 1 ELSE 0 END

    UPDATE Tbl_SendItemList
       SET SumPrice = @SumPrice 
           ,SendItemCount = @SendItemCount
           ,SendMailCount = @SendMailCount
           ,LCreateTime = GETDATE()
           WHERE AccountID = @AccountID AND PlayerID = @PlayerID
    IF @@ROWCOUNT = 0
       INSERT Tbl_SendItemList(AccountID
                            ,PlayerID
                            ,SumPrice
                            ,SendItemCount
                            ,SendMailCount
                            ,FCreateTime
                            ,LCreateTime)
              SELECT @AccountID
                     ,@PlayerID
                     ,@SumPrice
                     ,@SendItemCount
                     ,@SendMailCount
                     ,GETDATE()
                     ,GETDATE()
END*/