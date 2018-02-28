-- =============================================
-- Author:		<ZhouCheng>
-- Create date: <20100623>
-- Description:	<批量导入文本文件数据到数据表>
-- =============================================
/*CREATE PROC [dbo].[Sp_GetData]
(
	@Path				VARCHAR(200),
	@DatabaseName		VARCHAR(200) = 'CHAR',
	@FileType			VARCHAR(10) = 'TXT',
	@FIELDTERMINATOR	VARCHAR(10) = '	'
)
AS
BEGIN
	DECLARE @SQL VARCHAR(MAX)
	SELECT @SQL = ''
	SELECT @SQL = 'EXEC XP_CMDSHELL ''dir '+@Path+''''
	CREATE TABLE #(DIR NVARCHAR(2000))
	INSERT # EXEC (@SQL)
	DECLARE @T TABLE (TBName VARCHAR(100))
	INSERT @T
	SELECT REVERSE(SUBSTRING(REVERSE(DIR),LEN(@FileType)+2,CHARINDEX(' ',REVERSE(DIR))-2-LEN(@FileType))) 
		FROM # WHERE DIR LIKE '%'+@FileType
	SELECT @SQL = @SQL+CHAR(13)+
	'BULK INSERT '+@DatabaseName+'.DBO.'+TBName+CHAR(13)+
	'FROM '''+@Path+TBName+'.'+@FileType+''''+CHAR(13)+
	'WITH ('+CHAR(13)+
			'FIELDTERMINATOR ='''+ @FIELDTERMINATOR+''','+CHAR(13)+
			'ROWTERMINATOR =''\n'','+CHAR(13)+
			'TABLOCK' + CHAR(13)+
		')' FROM @T
	PRINT @SQL
	ALTER DATABASE CHAR	SET RECOVERY BULK_LOGGED
	EXEC (@SQL)
	ALTER DATABASE CHAR	SET RECOVERY FULL
END*/

