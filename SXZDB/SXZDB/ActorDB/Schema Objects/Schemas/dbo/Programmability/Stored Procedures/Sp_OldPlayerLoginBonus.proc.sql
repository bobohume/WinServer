/*CREATE PROCEDURE [dbo].[Sp_OldPlayerLoginBonus] 
@AccountId		INT,			--帐号ID
@PlayerId		INT,			--角色ID
@PlayerName		VARCHAR(32)		--角色名称
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON

	DECLARE	@OldPlayerMsgContest	VARCHAR(2048)
	DECLARE	@OldPlayerTitle			VARCHAR(32)
	DECLARE @OldBonusItemId			INT
	DECLARE @OldBonusItemName		VARCHAR(32)
	DECLARE @ERR					INT

	SET @ERR = 0
	SET @OldBonusItemId = 119011069
	SET @OldBonusItemName = '赤磷金鱼'

	SET @OldPlayerMsgContest = '<t c=''0xff9900ff''>亲爱的仙友近来可好？欢迎您再次来到《神仙传》。' + 
								'新的宿命已经展开，特别赠送高级坐骑【</t><t c=''0x00ff00ff''>' + 
								CAST(@OldBonusItemId AS VARCHAR(12)) + 
								'</t><t c=''0xff9900ff''>】以助您此番修行顺利。' +
								'感谢您对《神仙传》的支持！《神仙传》运营团队</t>'
	SET @OldPlayerTitle = '新的宿命修行奖励'

	BEGIN TRANSACTION OldPlayerLoginBonus

	--判断是否老玩家
	IF NOT EXISTS( SELECT 1 FROM Tbl_Account WHERE AccountId = @AccountId AND [Status] != 1 )
	BEGIN
		INSERT INTO Tbl_MailList (sender,sender_name,[money],itemId,
			item_count,send_time,recver,recver_name,isSystem,title,msg)
			SELECT 0,'系统',0,@OldBonusItemId,1,GetDate(),@PlayerId,@PlayerName,
					1,@OldPlayerTitle,@OldPlayerMsgContest
		--老玩家领取过奖励后置2
		UPDATE Tbl_Account SET [Status] = 2 WHERE AccountId = @AccountId
	END

	IF @ERR = 0
		COMMIT TRANSACTION OldPlayerLoginBonus
	ELSE
		ROLLBACK TRANSACTION OldPlayerLoginBonus
END
*/