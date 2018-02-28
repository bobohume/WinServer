-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-07>
-- Description:	<更新登录信息数据>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdateLoginTime]
@AccountID	INT
AS
BEGIN
	SET NOCOUNT ON
	UPDATE Tbl_Account SET LoginTime=GetDate() WHERE AccountID= @AccountID
END
