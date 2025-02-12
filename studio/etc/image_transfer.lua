--[[
================================================================================================
METER READING EXAMPLE
]]

local cosem = require("cosem")
local util = require("util")

--[[ Prints the clock in a human readable format
     Argument must be an OctetString structure

OCTET STRING (SIZE(12))
{
	year highbyte, 
	year lowbyte,
	month,
	day of month,
	day of week,
	hour,
	minute,
	second,
	hundredths of second,
	deviation highbyte,
	deviation lowbyte,
	clock status
}
	 
]]
function print_clock(clk)
	if clk.cosem_type == "OCTETSTRING" then
		
		local array = util.split(clk.data, ";")
		local days = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" } 
        function dow(rd) 
                return days[rd % 7 + 1] 
        end 
		
		-- Then decode and print the date
		local year = (tonumber(array[1]) << 8) +  tonumber(array[2])
		local month = tonumber(array[3])
		local day = tonumber(array[4])
		local hours = tonumber(array[6])
		local minutes = tonumber(array[7])
		local seconds = tonumber(array[8])
		
		print(year.."/"..month.."/"..day.." "..dow(day).." "..hours..":"..minutes..":"..seconds)
		
	else
		print("Not a clock object")
	end
end

function get_clock()
	local object = Value:new("OBIS")
    object.code = "8-0 0 1 0 0 255-2";

    -- get the current value of the clock
    local retCode, str = getCosem(0, object.code);
	
    if retCode == 0 then
		object:append(Value:from_lua_string(str))
    else
        print("Cannot read meter!")
    end
	
	return object;
end

-- Connect to the meter
connect(0)
-- Actually get the clock ; the value returned is in our internal generic array format
local dt = get_clock()
print(util.dump(dt))
print_clock(dt.data[1])
-- Disconnect from the meter
disconnect(0)

print("coucou")
