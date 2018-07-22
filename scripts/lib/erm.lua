
DATA = DATA or {}

local ERM

DATA.ERM = DATA.ERM or {}
ERM = DATA.ERM

ERM.flag = ERM.flag or {}
ERM.v = ERM.v or {}
ERM.z = ERM.z or {}

local y = {}

ERM.getY = function(key)
	y[key] = y[key] or {}
	return y[key]
end

local __IF = nil

ERM.IF = function(...)
	__IF = __IF or require("core:erm.IF")
	__IF.ERM = ERM
	return __IF
end

ERM.callInstructions = function(cb)
	if not ERM.instructionsCalled then
		cb()
		ERM.instructionsCalled = true
	end

end

return ERM
