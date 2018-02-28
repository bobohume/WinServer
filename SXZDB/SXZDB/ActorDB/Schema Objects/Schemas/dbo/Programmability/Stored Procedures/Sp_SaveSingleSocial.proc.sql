-- =============================================
-- Author:		<Daniel>
-- Create date: <2011-03-11>
-- Description:	<单条保存社会关系>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_SaveSingleSocial] 
@PlayerID		INT,
@FriendID		INT,
@TYPE			TINYINT,
@FriendValue	INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	BEGIN TRANSACTION SaveSingleSocial

    UPDATE TBL_SOCIAL SET FriendValue=@FriendValue WHERE player=@FriendID AND target=@PlayerID
    INSERT INTO TBL_SOCIAL (Player,Target,[type],friendvalue)
			VALUES (@PlayerID, @FriendID, @Type, @FriendValue)

	COMMIT TRANSACTION SaveSingleSocial
END
