-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-07>
-- Description:	<更新角色登录信息数据>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdatePlayerLoginTime]
@PlayerID	INT
AS
BEGIN
	SET NOCOUNT ON
	UPDATE Tbl_Player SET LastLoginTime=GetDate(), LastUpdateTime=GetDate() WHERE PlayerID=@PlayerID
END
