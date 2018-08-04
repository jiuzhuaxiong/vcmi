local BU = {}

local BU_C = function(x, p1)
	assert(type(p1) == "nil", "BU:C can only check value")

	local battle = BATTLE

	ret = battle:battleIsFinished()

	if type(ret) == "nil" then
		return 0
	else
		return 1
	end
end

BU.C = function(self, x, ...)
	local argc = select('#', ...)

	if argc == 1 then
		return BU_C(x, ...)
	end
end

return BU
