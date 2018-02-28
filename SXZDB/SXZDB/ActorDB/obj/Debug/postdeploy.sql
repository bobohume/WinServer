/*
后期部署脚本模板							
--------------------------------------------------------------------------------------
此文件包含将附加到生成脚本中的 SQL 语句		
使用 SQLCMD 语法将文件包含到后期部署脚本中			
示例:      :r .\myfile.sql								
使用 SQLCMD 语法引用后期部署脚本中的变量		
示例:      :setvar TableName MyTable							
               SELECT * FROM [$(TableName)]					
--------------------------------------------------------------------------------------
*/
--为地图编号布局配置

--为世界服务器运行提供缺省配置
IF NOT EXISTS(SELECT 1 FROM Tbl_World)
BEGIN
	INSERT INTO Tbl_World
           (TopLevel,TopDisaster,ActivityFlag,Val0,Val1,Val2,Val3,MaxStalls,TotalFlourish)
			VALUES(80,0,0,0,1,1,1,50,500)
END

GO
