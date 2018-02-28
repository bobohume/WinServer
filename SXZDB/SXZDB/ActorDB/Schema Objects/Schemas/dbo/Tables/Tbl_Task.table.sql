CREATE TABLE [dbo].[Tbl_Task] (
	[TaskId]			 INT																			NOT NULL,
    [PlayerID]			 INT																			NOT NULL, 
	[TaskVal]			 INT		CONSTRAINT [DF_Tbl_TASK_TaskVal]		DEFAULT((0))				NOT NULL,
	[FinishFlag]		 INT		CONSTRAINT [DF_Tbl_TASK_FinishFlag]		DEFAULT((0))				NOT NULL,
);