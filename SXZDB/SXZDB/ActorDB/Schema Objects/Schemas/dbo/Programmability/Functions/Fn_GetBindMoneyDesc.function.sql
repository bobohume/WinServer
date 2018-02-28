CREATE FUNCTION [dbo].[Fn_GetBindMoneyDesc](@Money INT)
RETURNS VARCHAR(300)
WITH EXECUTE AS CALLER
AS
BEGIN
	DECLARE @RET VARCHAR(300)
	DECLARE @GOLD INT,@SILVER INT, @COPPER INT
	SET @GOLD = @Money / 10000
	SET @SILVER = (@Money - @GOLD * 10000)/100
	SET @COPPER = @Money % 100
	SET @RET = ''

	IF @GOLD > 0
		SET @RET = @RET + '<t c=''0xffff22ff''>' + CAST(@GOLD AS VARCHAR(12)) +
		'</t><i s=''gameres/gui/images/GUIWindow31_1_024.png''/>'

	IF @SILVER > 0
		SET @RET = @RET + '<t c=''0xffff22ff''>' + CAST(@SILVER AS VARCHAR(12)) +
		'</t><i s=''gameres/gui/images/GUIWindow31_1_025.png''/>'

	IF @COPPER > 0
		SET @RET = @RET + '<t c=''0xffff22ff''>' + CAST(@COPPER AS VARCHAR(12)) +
		'</t><i s=''gameres/gui/images/GUIWindow31_1_026.png''/>'

	IF @GOLD=0 AND @SILVER=0 AND @COPPER=0
		SET @RET = '<t c=''0xffff22ff''>0</t><i s=''gameres/gui/images/GUIWindow31_1_024.png''/>' +
			'<t c=''0xffff22ff''>0</t><i s=''gameres/gui/images/GUIWindow31_1_025.png''/>' +
			'<t c=''0xffff22ff''>0</t><i s=''gameres/gui/images/GUIWindow31_1_026.png''/>'
	RETURN @RET
END