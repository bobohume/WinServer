-- =============================================
-- Author:		<Natural>
-- Create date: <2013-11-27>
-- Description:	<Appstore 后续操作没执行成功取消>
-- =============================================
CREATE PROCEDURE [dbo].[USP_CacleVertifyRecord]
@transaction_id		VARCHAR (32)

AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON

	DELETE TBL_PurhaseHistory WHERE transaction_id = @transaction_id;
	
END