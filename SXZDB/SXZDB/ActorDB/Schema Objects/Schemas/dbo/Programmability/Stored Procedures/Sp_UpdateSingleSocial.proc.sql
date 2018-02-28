-- =============================================
-- Author:		<Daniel>
-- Create date: <2011-03-11>
-- Description:	<单条更新社会关系>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdateSingleSocial] 
@PlayerID		INT,
@FriendID		INT,
@TYPE			TINYINT,
@FriendValue	INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	BEGIN TRANSACTION UpdateSingleSocial
	UPDATE TBL_SOCIAL SET [type]=@Type,FriendValue=@friendvalue
			WHERE Player=@PlayerID AND target=@FriendID
	UPDATE TBL_SOCIAL SET FriendValue=@FriendValue 
			WHERE player=@FriendID AND target=@PlayerID
	COMMIT TRANSACTION UpdateSingleSocial
END
