-- =============================================
-- Author:		<Daniel>
-- Create date: <2012-04-16>
-- Description:	<计算字符串的MD5值结果>
-- =============================================
CREATE FUNCTION [dbo].[UFN_MD5]
(@DATA VARCHAR (MAX))
RETURNS CHAR (32)
AS
BEGIN
	DECLARE @HASH CHAR(32)
	SET @HASH = SYS.FN_VARBINTOHEXSUBSTRING(0,HASHBYTES('MD5', @DATA),1,0)
	RETURN @HASH
END
