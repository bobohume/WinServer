-----------------------------------------------------------------------------------------------------
-----导入给策划使用的lua脚本接口
-----------------------------------------------------------------------------------------------------
dofile("script/globalFunction.lua");
--[[
注意一点：所有的这些成员数据只能参照，不能修改，修改要调用具体的c++代码，改掉内存和数据库
说明：Player player = getPlayer(nPlayerId);用来获取玩家对象，类型是Player,玩家数据是player， nPlayerId为玩家id
取得玩家数据以后Player中的isNil()用来判断玩家数据是否有效 。
local Player player = getPlayer(50000001);
if(player:isNil() == true) then
	-- todo
end
CMissionDataPtr与CMissionData为任务data：CMissionDataPtr是指针对象型的，用nil判断直接是否为空。CMissionData是对象型的。两个取成员函数方式一样
	成员数据用., 成员方法用:(stTimeSetPtr与stTimeSet一样)
getMissionData(iMissionId);获取任务data，返回类型是CMissionDataPtr类型。其成员与用法具体参照 AccountHandle中的 Export中的方法
例如：  
//获取700001任务的data
local CMissionDataPtr pMissionData = getMissionData(700001);
	if(pMissionData ~= nil) then
		pMissionData:printAllEmlem();
end
]]--

function testAllMission(plyaerId)
	local Player player = getPlayer(50000001);
	if(player:isNil() == true) then
		for i =0, 200 do
			local stTimeSetPtr pTimer = player:getMission(i);
			if(pTimer ~= nil) then
				print(pTimer .id);
			end
		end
	end
end
