package.loaded[ 'cosem' ] = nil -- Leave this line uncommented to force a module reloading, usefull when working on it...
local cosem = require("cosem")
local util = require("util")


function test_regex()
    local line = "     element OCTETSTRING,   value:0;1;"

    local OctetString = "OCTETSTRING"
    local Unsigned8 = "UNSIGNED8"
    local value = nil

    local data_type, value = string.match(line, "%s*element%s*(%w+),%s*value:(.*)")

    if data_type == Unsigned8 then
        print("Found:"..Unsigned8.." value: "..value)

    elseif data_type == OctetString then
        print("Found:"..OctetString.." value: "..value)

    else
        print("skipped")
    end
end


function load_cayox_file()
    local cayox_file = "tests/cayox1.txt"
    print("Trying to parse: "..cayox_file)

    local valid, object = cosem.parse_cayox_file(cayox_file)
    if valid then

    print(util.dump(object));

    print("-------------------------------------------------------------")
        print(object:to_string())
    else
      print("Bad Cayox input file")
    end
end

--load_cayox_file();

local value = "1;2;3;4;a;b;c;"

print(table.concat(string.unpack(";", value)))

