-- =============================================
-- Author:		<Daniel>
-- Create date: <2010-05-09>
-- Description:	<读取玩家完整数据>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_LoadPlayer]
@PlayerID INT		--账号ID
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @ERROR INT
	SET @ERROR = '0000'
	BEGIN TRANSACTION LoadPlayer
		
		

	COMMIT TRANSACTION UpdateTopRankCache
END



