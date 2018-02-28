function printTable(tt)

	if not tt then
		log("the table is nil");
		return
	end
	for k,v in pairs(tt) do
		if v ~= nil then
			log("%s--->%s",tostring(k),tostring(v));
		end
	end
end