                   -- Welcome to Shaddam! -- 
                   -------------------------

-- Introduction
----------------

-- Shaddam is the successor of Manitoo, a DLMS/Cosem test tool to test meters.
-- It brings a true scripting language to write your automated scripts: the Lua language

-- Visit the official website: http://www.lua.org


-- Examples
----------------
package.loaded[ 'cosem' ] = nil -- Leave this line uncommented to force a module reloading, usefull when working on it...
local cosem = require("cosem")
local util = require("util")

function get_current_tariff()

    local currentTariffObj = "1-0 0 96 14 0 255-2"
    local retCode, currentTariff = getCosem(currentTariffObj);
    print("Current Tariff")

    if retCode == 0 then
        print("AxDR type="..string.byte(currentTariff, 2))
        print("Value="..string.byte(currentTariff, 3))
        if (string.byte(currentTariff, 3)) ~= 3 then 
            print("currentTariff is not equal to tariff 3")
        end
    else
        print("Cannot read meter!")
    end

end

function get_clock()

    local clockObj = "8-0 0 1 0 0 255-2"
    -- get the current value of the clock
    local retCode, dt = getCosem(clockObj);
    print("")
    print("Reading clock ...")

    if retCode == 0 then
        print("AxDR type: "..string.byte(dt,1))
	local size = string.byte(dt,2)
	print("Size: "..size)
        print("Value=")
        for i=0, size, 1 do
            print(string.byte(dt,3 + i))
        end
    else
        print("Cannot read meter!")
    end

end

--[[
================================================================================================
CAYOX FILE LOADING EXAMPLE
]]

function load_cayox_file()
	local cayox_file = "tests/cayox1.txt"
	print("Trying to parse: ".."tests/cayox1.txt")

    local valid, object = cosem.parse_cayox_file(cayox_file)
    if valid then

	print(util.dump(object));

	print("-------------------------------------------------------------")
        print(object:to_string())
    else
	  print("Bad Cayox input file")
    end
end

--[[
================================================================================================
MEMORY DATA STRUCTURE EXAMPLES
]]



function data_structure_example()
	
	print("-------------------------------------------------------------")

	local str1 = cosem.value("OCTETSTRING", "coucou")
	local object = cosem.value("STRUCTURE", {})
	
	object:append(str1)
	
	print(util.dump(object));

	print(object:to_string())


end

--[[
================================================================================================
METER READING EXAMPLE
]]

-- Connect to the meter
--connect();
-- Actually get the clock ; the value returned is in our internal generic array format
--local dt = get_clock();
-- Disconnect from the meter
--disconnect();


-- This example will load a Cayox data file into memory and print it 
load_cayox_file();

--data_structure_example()

--[[
================================================================================================
LOOP EXAMPLE
]]

function loop_example()

    local x = os.clock()
    local f = assert(io.open("out.txt", "w"))
    local i = 0

    repeat
        f:write(i.."\n")    
        delay_ms(1000);
        i = i+1
        print(i.."\n")
    until (os.clock()-x) >= 4

    f:close()
    print("end loop")
end



-- End of file
