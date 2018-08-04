require("battle.Unit")

local battle = BATTLE

local BU = {}

local BU_C = function(x, p1)
	assert(type(p1) == "nil", "BU:C can only check value")

	ret = battle:isFinished()

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

local BU_D = function(x, hex, p1)
	assert(type(p1) == "nil", "BU:D can only check value")

	unit = battle:getUnitByPos(hex, false)

	if unit then
		if unit:isAlive() then
			return nil, -2
		else
			return nil, unit:unitId()
		end
	else
        return nil, -1
	end

end

BU.D = function(self, x, ...)
	local argc = select('#', ...)

	if argc == 2 then
		return BU_D(x, ...)
	end
end

local BU_E = function(x, hex, p1)
	assert(type(p1) == "nil", "BU:E can only check value")

	unit = battle:getUnitByPos(hex, false)

	if unit and unit:isAlive() then
		return nil, unit:unitId()
	else
		return nil, -1
	end
end

BU.E = function(self, x, ...)
	local argc = select('#', ...)

	if argc == 2 then
		return BU_E(x, ...)
	end
end

return BU
