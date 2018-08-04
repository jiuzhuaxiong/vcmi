
DATA = DATA or {}

local ERM = {}
local DATA = DATA

DATA.ERM = DATA.ERM or {}

DATA.ERM.flag = DATA.ERM.flag or {}
DATA.ERM.quick = DATA.ERM.quick or {}
DATA.ERM.v = DATA.ERM.v or {}
DATA.ERM.z = DATA.ERM.z or {}

ERM.flag = DATA.ERM.flag
ERM.quick = DATA.ERM.quick
ERM.v = DATA.ERM.v
ERM.z = DATA.ERM.z

local y = {}

ERM.getY = function(key)
	y[key] = y[key] or {}
	return y[key]
end

local __BU
local __IF

ERM.BU = function(...)
	__BU = __BU or require("core:erm.BU")
	__BU.ERM = ERM
	return __BU
end

ERM.IF = function(...)
	__IF = __IF or require("core:erm.IF")
	__IF.ERM = ERM
	return __IF
end

local triggers = {}

ERM.addTrigger = function(t)
	triggers[t.name] = triggers[t.name] or {}
	table.insert(triggers[t.name], t.fn)
end

ERM.callInstructions = function(cb)
	if not DATA.ERM.instructionsCalled then
		cb()
		ERM.callTrigger("PI")
		DATA.ERM.instructionsCalled = true
	end

end

ERM.callTrigger = function(name)

	if triggers[name] then
		for _, fn in ipairs(triggers[name]) do
			fn()
		end

	end

end

return ERM
