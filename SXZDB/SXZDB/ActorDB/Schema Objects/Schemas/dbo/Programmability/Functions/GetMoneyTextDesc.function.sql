CREATE FUNCTION [dbo].[GetMoneyTextDesc](@Money INT)
RETURNS VARCHAR(256)
WITH EXECUTE AS CALLER
AS
BEGIN
	DECLARE @RET VARCHAR(256)
	DECLARE @Gold INT, @Sliver INT, @Copper INT
	SET @Gold = @Money / 10000
	SET @Sliver = (@Money - @Gold * 10000) / 100
	SET @Copper = @Money % 100
	SET @RET = '<t c=''0xffff22ff''>' + CAST(@Gold AS VARCHAR(12)) +
		'</t><i s=''gameres/gui/images/GUIWindow31_1_024.png''/><t c=''0xffff22ff''>' +
		CAST(@Sliver AS VARCHAR(2)) +
		'</t><i s=''gameres/gui/images/GUIWindow31_1_025.png''/><t c=''0xffff22ff''>' +
		CAST(@Copper AS VARCHAR(2)) +
		'</t><i s=''gameres/gui/images/GUIWindow31_1_026.png''/>'
	RETURN @RET
END