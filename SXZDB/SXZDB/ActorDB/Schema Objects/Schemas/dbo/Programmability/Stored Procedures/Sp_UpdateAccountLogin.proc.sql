-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-07>
-- Description:	<更新帐号登录数据>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdateAccountLogin]
@AccountID		INT,
@LoginIP		VARCHAR(20),
@LoginError		INT
AS
BEGIN
	SET NOCOUNT ON
	UPDATE Tbl_Account SET LoginTime=GetDate(),	LoginIP=@LoginIP, LoginError=@LoginError
			WHERE AccountID = @AccountID
END


