CREATE PROC Sp_QueryPlayerInfo
@PlayerID  INT     --角色ID
AS
BEGIN
	SET NOCOUNT ON
	SELECT AccountID,PlayerID,PlayerName,[Level]
		FROM Tbl_Player WHERE PlayerID = @PlayerID
END