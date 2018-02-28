-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-8-27>
-- Description:	<删除角色>
-- =============================================
/*CREATE PROCEDURE  [dbo].[Sp_DeletePlayer]
@PlayerID	INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	BEGIN TRANSACTION DeletePlayer
	DELETE FROM Tbl_Player_Deleted WHERE PlayerID=@PlayerID
	/*为了确保不误删，首先备份角色信息*/
	INSERT INTO Tbl_Player_Deleted 
		SELECT AccountId, PlayerId, PlayerName, GetDate(), DBVersion, Sex, [Level], Family,FamilyPos,[FirstClass],
				FaceImage, MasterLevel, Activity,MPMStudyLevel,LastLoginTime, LastLogoutTime, BLOBData
			FROM Tbl_Player WHERE PlayerID=@PlayerID
	/*删除角色信息*/
	DELETE FROM Tbl_Player WHERE PlayerID = @PlayerID
	COMMIT TRANSACTION DeletePlayer
END*/







