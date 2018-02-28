CREATE PROCEDURE [dbo].[Sp_UpdateAccountLogoutTime]
@AccountID		INT,
@logoutTime		DATETIME
AS
BEGIN
	SET NOCOUNT ON
	UPDATE Tbl_Account SET LogoutTime=@logoutTime WHERE AccountID=@AccountID
END