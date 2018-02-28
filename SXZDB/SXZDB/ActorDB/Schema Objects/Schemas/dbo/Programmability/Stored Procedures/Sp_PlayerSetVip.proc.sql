-- =============================================
-- Author:		<LivenHotch>
-- Create date: <2010-05-27>
-- Description:	<内购不在线加钱处理>
-- modifyer:    <Daniel>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_PlayerSetVip]
@GMPLAYERID		INT,
@PLAYERID		INT,
@VIP			INT,
@REASON			VARCHAR(64)
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @Err INT
	SET @Err = 0
	DECLARE @VipLv INT
	SET @VipLv = 0

	BEGIN TRANSACTION PlayerSetVip
	SELECT @VipLv=VipLv FROM Tbl_Player WHERE PlayerID = @PLAYERID 
	IF @@ROWCOUNT <> 0
	BEGIN
		UPDATE Tbl_Player  SET VipLv = @VIP WHERE PlayerID = @PLAYERID

		INSERT INTO Tbl_LogPlayer(sourceid,targetid,opttype,val0,val1,val2,val3,val4) 
			VALUES (@GMPLAYERID, @PLAYERID,'提升VIP等级',@VIP,@VipLv,@REASON,'','')
	END
	ELSE
		SET @Err = 1

	IF @Err <> 0
		ROLLBACK TRANSACTION PlayerSetVip
	ELSE
		COMMIT TRANSACTION PlayerSetVip

	SELECT @Err AS 'result'

END