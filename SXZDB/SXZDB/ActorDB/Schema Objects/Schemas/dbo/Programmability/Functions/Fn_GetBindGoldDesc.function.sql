CREATE FUNCTION [dbo].[Fn_GetBindGoldDesc](@BindGold INT)
RETURNS VARCHAR(128)
WITH EXECUTE AS CALLER
AS
BEGIN
	DECLARE @RET VARCHAR(128)
	SET @RET = '<t c=''0xffff22ff''>' + CAST(@BindGold AS VARCHAR(12)) +
		'</t><i s=''gameres/gui/images/GUIWindow31_1_028.png''/>'
	RETURN @RET
END