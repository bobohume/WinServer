-- =============================================
-- Author:		<Daniel>
-- Create date: <2011-03-11>
-- Description:	<删除某好友的社会关系>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_DeleteTargetSocial] 
@PlayerID		INT,
@FriendID		INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	BEGIN TRANSACTION DeleteTargetSocial
	DELETE FROM TBL_SOCIAL WHERE Player=@PlayerID AND target=@FriendID
	UPDATE TBL_SOCIAL SET FriendValue=0 WHERE Player=@FriendID AND target=@PlayerID
	COMMIT TRANSACTION DeleteTargetSocial
END
