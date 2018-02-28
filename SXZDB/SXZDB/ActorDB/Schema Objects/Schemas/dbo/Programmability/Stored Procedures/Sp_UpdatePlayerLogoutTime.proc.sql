-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-07>
-- Description:	<更新角色登出信息数据>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdatePlayerLogoutTime]
@PlayerID	INT
AS
BEGIN
	SET NOCOUNT ON
	UPDATE Tbl_Player SET LastLogoutTime=GetDate(),LastUpdateTime=GetDate() WHERE PlayerID= @PlayerID
END
