
-- Script.lua called by WPKG-gp in order to execute WPKG and print/log output

-- Available variables:
-- Set by Group Policy:
-- * WpkgPath       - Path to WPKG.js script or to batch file running WPKG
-- * WpkgParameters - Arguments to WPKG.js or batch file running WPKG
-- * WpkgVerbosity  - Either "low", only print normal messages, "medium"
--   which means additional logging to Event Log, or "high" which outputs
--   all data from WPKG
-- Set by calling program
-- * CallingMethod  - Either "script" or "gpe".
--   * "script" means that it is called by debugging program, and it should
--     print() messages instead of calling WriteStatus()
--   * "gpe" means that it is called by the Group Policy Subsystem (from
--     WPKG-gp.dll) and that it should print output by using  WriteStatus()

-- Available functions:
-- WriteStatus(String)    - Writes string by using callbackfunction of GPO
--                          Subsystem
-- WriteLog(Flag, String) - Writes string to Event Log with the Flag Set
--                          Flags are made available by calling SetLogFlags()
-- SetLogFlags()          - Initializes the global variables to be used by
--                          WriteLog():
--  * EVENTLOG_SUCCESS
--  * EVENTLOG_AUDIT_FAILURE
--  * EVENTLOG_AUDIT_SUCCESS
--  * EVENTLOG_ERROR_TYPE
--  * EVENTLOG_INFORMATION_TYPE
--  * EVENTLOG_WARNING_TYPE

-- Configuring global variables
SetLogFlags()
LOG_TYPE_PRINT   = 0
LOG_TYPE_LOG     = EVENTLOG_INFORMATION_TYPE
LOG_TYPE_WARNING = EVENTLOG_WARNING_TYPE
LOG_TYPE_ERROR   = EVENTLOG_ERROR_TYPE

LOG_LEVEL_INFO    = 0
LOG_LEVEL_VERBOSE = 1
LOG_LEVEL_DEBUG   = 2


WPKGCOMMAND = ""
G_LOG_LEVEL = 1


function Log(LogType, LogLevel, String)
	-- If called on boot
	if CallingMethod == "gpe" then
		-- Call WriteStatus for LOG_TYPE_PRINT
		if LogType == LOG_TYPE_PRINT then
			-- Write all of LOG_LEVEL_INFO
			if LogLevel == LOG_LEVEL_INFO then
				WriteStatus(String)
			else
				WriteStatus(String)
			end
		end

		-- Log everything else to Event log at appropriate level
		if LogLevel <= G_LOG_LEVEL and LogLevel ~= LOG_TYPE_PRINT then
			WriteLog(LogType, String)
		end

	-- If called as "script"
	else
		-- Show messages prefixed by level:
		print("Log Type " .. LogType .. " and Log Level " .. LogLevel  .. ": " .. String)
		if LogLevel <= G_LOG_LEVEL and LogType ~= LOG_TYPE_PRINT then
			WriteLog(LogType, String)
		end
	end
end



-- Checks if variables are set and initializes
function Initialize ()

	-- Convert WpkgVerbosity to the global number variable LogLevel
	-- Default is 1, which is set already when initializing the global variable
	if (type(WpkgVerbosity) ~= "string" or WpkgVerbosity == "") then
		Log(LOG_TYPE_WARNING, LOG_LEVEL_DEBUG, "Group Policy setting WpkgVerbosity is not set, setting Log level to 1")
	else
		G_LOG_LEVEL = tonumber(WpkgVerbosity)
		Log(LOG_TYPE_LOG, LOG_LEVEL_DEBUG, "Group Policy setting WpkgVerbosity is set to: " .. WpkgVerbosity)
	end


	-- Check WPKGPath, must be set
	if (type(WpkgPath) ~= "string" or WpkgPath == "") then
		Log(LOG_TYPE_LOG, LOG_LEVEL_ERROR, "Group policy setting WpkgPath is not set, aborting")
		return 1
	else
		Log(LOG_TYPE_LOG, LOG_LEVEL_DEBUG, "Group policy setting WpkgPath set to: " .. WpkgPath)
	end


	-- Check WpkgParameters, don't have to be set
	if (type(WpkgParameters) ~= "string" or WpkgParameters == "") then
		Log(LOG_TYPE_LOG, LOG_LEVEL_DEBUG, "Group Policy setting WpkgParameters is not set, ignoring")
		WPKGCOMMAND = WpkgPath
	else
		Log(LOG_TYPE_LOG, LOG_LEVEL_DEBUG, "Group Policy setting WpkgParameters set to: " .. WpkgParameters)
		WPKGCOMMAND = WpkgPath .. " " .. WpkgParameters
	end
	Log(LOG_TYPE_LOG, LOG_LEVEL_DEBUG, "Setting WpkgCommand to: " .. WPKGCOMMAND)
end

function StatusRemoveTimestamp(line)
	-- removing timestamp
	local i,j = string.find(line, "%d+-%d%d%-%d%d %d+:%d%d:%d%d.*, STATUS  : ")
	-- Has no timestamp
	if i == nil then
		return nil
	end

	return string.sub(line, j+1)
end

function StatusGetStage(line)
	local tmpline = StatusRemoveTimestamp(line)
	if tmpline == nil then
		return nil
	end

	local i,j = string.find(tmpline, "^.+:")
	if  i == nil then
		return nil
	end
	return string.sub(tmpline, i, j-1)
end

function StatusGetOperation(line)
	local tmpline = StatusRemoveTimestamp(line)
	if tmpline == nil then
		return nil
	end

	local i,j = string.find(tmpline, ":.* '")
	if i == nil then
		return nil
	end
	return string.sub(tmpline, i+2, j-2)
end

function StatusGetPackageName(line)
	-- removing the progress Info
	i,j = string.find(line, "' %(%d+/%d+%)$")
	if i == nil then
		return nil
	end

	tmpline = string.sub(line, 0, i)
	i,j = string.find(tmpline, "'.*'$")
	if i == nil then
		return nil
	end
	return string.sub(tmpline, i+1, j-1)
end

-- returns percent, currentnumber, totalnumber on success
-- returns nil on error
function StatusGetPercent(line)
	local i,j = string.find(line, "%d+/%d+%)$")
	if i == nil then
	  return nil
	end

	local tmpline = string.sub(line, i,j-1)
	i,j = string.find(tmpline, "%d+/")
	if i == nil then
		return nil
	end
	currentnumber = tonumber(string.sub(tmpline, i, j-1))
	totalnumber   = tonumber(string.sub(tmpline, j+1))
	percent = tonumber(string.format("%.0f", (currentnumber/totalnumber) * 100))
	return percent, currentnumber, totalnumber
end

-- returns "StatusInfo" if line has timestamp
-- returns "InfoLine" if not
-- returns nil, if it is a "Usernotification suppressed line
function IsnfoOrStatusLine(line)
	-- Looking for timestamp at beginning
	local i,j = string.find(line, "%d+-%d%d%-%d%d %d+:%d%d:%d%d.*,")

	-- Has no timestamp
	if i == nil then
		-- Always ignore linse starting with "User notification suppressed" lines
		if string.find(line, "^User notification suppressed.") ~= nil then
			return nil
		end
		return "InfoLine", line
	end

	return "StatusInfo"
end


function InfoGetRelevantLine(line)
	local i = string.find(line, "^Installing")
	local j = string.find(line, "^Upgrading")
	local k = string.find(line, "^Removing")
	if i ~= nil or j ~= nil or k ~= nil then
		return line
	end
	return nil
end

function ParseWpkgOutput(line, percent)
	local infoline
	local curpercent = StatusGetPercent(line)
	local messageType = IsnfoOrStatusLine(line)
	local currentstatusoperation = StatusGetOperation(line)
	local currentinfo = InfoGetRelevantLine(line)

	if curpercent ~= nil then
		percent = curpercent
	end
	if currentstatusoperation ~= nil then
		infoline = currentstatusoperation .. "s"
	elseif currentinfo ~= nil then
		infoline = currentinfo
	end

	return percent, infoline
end

function WpkgWorker()
	local line
	local msg = ""
	local percent = 0
	local infoline
	Log(LOG_TYPE_LOG, LOG_LEVEL_DEBUG, "Executing: " .. WPKGCOMMAND)

	p = io.popen(WPKGCOMMAND)
	for line in p:lines() do
		percent, infoline = ParseWpkgOutput(line, percent)
		if infoline ~= nil then
			msg = infoline
		end

		Log(LOG_TYPE_PRINT, LOG_LEVEL_INFO, msg .. " " .. percent .."%")
	end
	p:close()
end


Initialize()
WpkgWorker()
