CREATE FUNCTION [dbo].[Fn_GetGoldDesc](@Gold INT)
RETURNS VARCHAR(128)
WITH EXECUTE AS CALLER
AS
BEGIN
	DECLARE @RET VARCHAR(128)
	SET @RET = '<t c=''0xffff22ff''>' + CAST(@Gold AS VARCHAR(12)) +
			'</t><i s=''gameres/gui/images/GUIWindow31_1_027.png''/>'
	RETURN @RET
END