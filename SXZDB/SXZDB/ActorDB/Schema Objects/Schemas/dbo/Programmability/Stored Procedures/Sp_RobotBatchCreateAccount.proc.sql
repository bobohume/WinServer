-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-06-05>
-- Description:	<批量创建帐号>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_RobotBatchCreateAccount]
@prename		VARCHAR(50),	--帐号前缀
@gmflag			INT,			--GM标志
@start			INT,			--帐号名后缀起始数
@end			INT				--帐号名后缀终止数
AS
BEGIN
	SET NOCOUNT ON
	DECLARE @AccountName VARCHAR(50)
	DECLARE @Num INT, @AccountID INT
	SELECT @Num = @end - @start + 1
	WHILE (@start <= @end)
	BEGIN
		SET @AccountName = @prename + CAST(@start AS VARCHAR(10)) 
		INSERT INTO TBL_AccountHelper ([name]) VALUES (@AccountName)
		SET @AccountId = @@Identity
		INSERT dbo.Tbl_Account(AccountName,AccountID,isadult,gmflag)
			VALUES(@AccountName, @AccountID,1,@gmflag)
		SET @start = @start + 1
	END
END