-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-12>
-- Description:	<更新玩家角色数据>
-- =============================================
/*CREATE PROCEDURE [dbo].[Sp_UpdatePlayer]
@PlayerID	INT,
@dbVersion	INT
AS
BEGIN
	SET NOCOUNT ON
	UPDATE Tbl_Player SET dbVersion=@dbVersion, LastUpdateTime=GetDate() WHERE PlayerID=@PlayerID
END*/