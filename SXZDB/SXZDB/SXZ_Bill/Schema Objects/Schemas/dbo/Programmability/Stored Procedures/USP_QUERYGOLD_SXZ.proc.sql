-- =============================================
-- Author:		Daniel
-- Create date: <2011-3-1>
-- Description:	<查询帐号充值元宝>
-- =============================================
CREATE PROCEDURE [dbo].[USP_QUERYGOLD_SXZ]
@AccountID INT
AS
BEGIN
	SET NOCOUNT ON
	SELECT PAY_POINTS FROM T_USER_PAY WHERE ACCOUNTID = @AccountID
END






