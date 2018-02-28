CREATE TABLE [dbo].[Tbl_OrgMember] (
    [PlayerId]		INT																			    NOT NULL,
    [OrgId]			INT			CONSTRAINT	[DF_Tbl_OrgMember_OrgId]			DEFAULT((0))		NOT NULL,
    [PlayerName]	VARCHAR(32)	CONSTRAINT	[DF_Tbl_OrgMember_PlayerName]		DEFAULT((''))		NOT NULL,
    [Contribute]    INT			CONSTRAINT	[DF_Tbl_OrgMember_Contribute]		DEFAULT((0))		NOT NULL,
	[GiftExp]		INT			CONSTRAINT	[DF_Tbl_OrgMember_GiftExp]			DEFAULT((0))		NOT NULL,
	[MemberLevel]   INT			CONSTRAINT	[DF_Tbl_OrgMember_MemberLevel]		DEFAULT((0))		NOT NULL,
	[MemberStatus]  INT			CONSTRAINT	[DF_Tbl_OrgMember_MemberStatus]		DEFAULT((0))		NOT NULL,
	[BattleVal]		INT			CONSTRAINT	[DF_Tbl_OrgMember_BattleVal]		DEFAULT((0))		NOT NULL,
	[Vip]			INT			CONSTRAINT	[DF_Tbl_OrgMember_Vip]				DEFAULT((0))		NOT NULL,
	[PlayerFace]	INT			CONSTRAINT	[DF_Tbl_OrgMember_PlayerFace]		DEFAULT((0))		NOT NULL,
	[LeaveTime]		DATETIME	CONSTRAINT	[DF_Tbl_OrgMember_LeaveTime]		DEFAULT((getdate()))NOT NULL,
);