-- =============================================
-- Author:		<Daniel>
-- Create date: <2011-03-11>
-- Description:	<两条更新社会关系>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdateDoubleSocial] 
@PlayerID		INT,
@FriendID		INT,
@TYPE			TINYINT,
@DestType       TINYINT,
@FriendValue	INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	BEGIN TRANSACTION UpdateDoubleSocial
	UPDATE TBL_SOCIAL SET [type]=@Type,FriendValue=@friendvalue
			WHERE Player=@PlayerID AND target=@FriendID
	UPDATE TBL_SOCIAL SET [type]=@DestType,FriendValue=@FriendValue 
			WHERE player=@FriendID AND target=@PlayerID
	COMMIT TRANSACTION UpdateDoubleSocial
END
