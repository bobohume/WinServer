-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-07>
-- Description:	<更新登出信息数据>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdateLogoutTime]
@AccountID			INT,
@CostTime			INT,
@dTotalOfflineTime	INT,
@dTotalOnlineTime	INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @TotalOnlineTime BIGINT
	BEGIN TRANSACTION UPDATELOGOUTTIME
	SELECT @TotalOnlineTime=TotalOnlineTime FROM Tbl_Account WHERE AccountID= @AccountID
	SET @TotalOnlineTime = @TotalOnlineTime + @CostTime
	IF @TotalOnlineTime <= 0x7FFFFFFF AND @TotalOnlineTime >= 0
	BEGIN
		UPDATE Tbl_Account SET
			TotalOnlineTime=@TotalOnlineTime,
			dTotalOnlineTime=@dTotalOnlineTime,
			dTotalOfflineTime=@dTotalOfflineTime
		WHERE AccountID= @AccountID
	END
	COMMIT TRANSACTION UPDATELOGOUTTIME
END
