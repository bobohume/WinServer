CREATE PROCEDURE [dbo].[Sp_SaveWorldSetting]
@TopLevel      INT,
@TopDisaster   INT,
@ActivityFlag  INT,
@val0		   INT,
@val1		   INT,
@val2		   INT,
@val3		   INT,
@val4		   INT,
@val5		   INT,
@val6		   INT,
@val7		   INT,
@MaxStalls     SMALLINT,
@TotalFlourish INT,
@Years5Assure  TINYINT,
@GodGivePath   TINYINT

AS
BEGIN
	SET NOCOUNT ON
	IF EXISTS (SELECT 1 FROM Tbl_World)
		UPDATE Tbl_World SET TopLevel=@TopLevel,TopDisaster=@TopDisaster,ActivityFlag=@ActivityFlag,
               val0=@val0,val1=@val1,val2=@val2,val3=@val3,val4=@val4,val5=@val5,val6=@val6,val7=@val7,
               MaxStalls=@MaxStalls,TotalFlourish=@TotalFlourish,Years5Assure=@Years5Assure,GodGivePath=@GodGivePath
	ELSE
		INSERT INTO Tbl_World (TopLevel,TopDisaster,ActivityFlag,val0,val1,val2,val3,val4,
                    val5,val6,val7,MaxStalls,TotalFlourish,Years5Assure,GodGivePath) VALUES (@TopLevel,@TopDisaster,
					@ActivityFlag,@val0,@val1,@val2,@val3,@val4,@val5,@val6,@val7,@MaxStalls,@TotalFlourish,@Years5Assure,@GodGivePath)
END