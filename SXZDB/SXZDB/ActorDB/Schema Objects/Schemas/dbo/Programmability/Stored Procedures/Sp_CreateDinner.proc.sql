CREATE PROCEDURE [dbo].[Sp_CreateDinner]
@PlayerID0	INT,
@PlayerID1  INT,
@PlayerID2	INT,
@PlayerID3	INT,
@PlayerID4	INT,
@PlayerName0 VARCHAR(32),
@PlayerName1 VARCHAR(32),
@PlayerName2 VARCHAR(32),
@PlayerName3 VARCHAR(32),
@PlayerName4 VARCHAR(32),
@PID0	INT,
@PID1	INT,
@PID2	INT,
@PID3	INT,
@PID4	INT,
@CreateTime INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @ERROR CHAR(4)
	SET @ERROR = '0000'
	DECLARE @ID INT
	SET @ID = -1

	BEGIN TRANSACTION CreateDinner
	
	if @ERROR = '0000'
	begin
		INSERT INTO Tbl_Dinner(PlayerID0, PlayerID1, PlayerID2, PlayerID3, PlayerID4, PlayerName0, PlayerName1, PlayerName2, PlayerName3, PlayerName4, CreateTime, PrincessID0, PrincessID1, PrincessID2, PrincessID3, PrincessID4) 
			SELECT @PlayerID0, @PlayerID1, @PlayerID2, @PlayerID3, @PlayerID4, @PlayerName0, @PlayerName1, @PlayerName2, @PlayerName3, @PlayerName4, @CreateTime, @PID0, @PID1, @PID2, @PID3, @PID4

		if @@ROWCOUNT = 0
			set @ERROR = '0006'
		else
			SET @ID = @@IDENTITY

	end
	
	if @ERROR <> '0000'
		ROLLBACK TRANSACTION CreateDinner
	else
		COMMIT TRANSACTION CreateDinner

	SELECT @ID, @CreateTime
END



