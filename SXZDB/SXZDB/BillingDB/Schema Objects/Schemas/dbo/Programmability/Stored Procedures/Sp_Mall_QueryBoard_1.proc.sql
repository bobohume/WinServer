CREATE PROCEDURE [dbo].[Sp_Mall_QueryBoard]

AS
BEGIN
	SET NOCOUNT ON
	SELECT MsgId, [Content], UpdValue FROM Tbl_Board
END