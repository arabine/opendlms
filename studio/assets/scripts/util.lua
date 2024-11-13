local util = {}


function table_print (tt, indent, done)
  done = done or {}
  indent = indent or 0
  if type(tt) == "table" then
    local sb = {}
    for key, value in pairs (tt) do
      table.insert(sb, string.rep (" ", indent)) -- indent it
      if type (value) == "table" and not done [value] then
        done [value] = true
        table.insert(sb, tostring (key).." = {\n");
        table.insert(sb, table_print (value, indent + 2, done))
        table.insert(sb, string.rep (" ", indent)) -- indent it
        table.insert(sb, "}\n");
      elseif "number" == type(key) then
        table.insert(sb, string.format("\"%s\"\n", tostring(value)))
      else
        table.insert(sb, string.format(
            "%s = \"%s\"\n", tostring (key), tostring(value)))
       end
    end
    return table.concat(sb)
  else
    return tt .. "\n"
  end
end


-- This utility function generates a basic "dump" of any kind of object
function util.dump( tbl )
    if  "nil"       == type( tbl ) then
        return tostring(nil)
    elseif  "table" == type( tbl ) then
        return table_print(tbl)
    elseif  "string" == type( tbl ) then
        return tbl
    else
        return tostring(tbl)
    end
end

-- like C strtok, splits on one more delimiter characters (finds every string not containing any of the delimiters)
-- example: var elements = split("bye# bye, miss$ american@ pie", ",#$@ ") -- returns "bye" "bye" "miss" "american" "pie"
function util.split(source, delimiters)
	local elements = {}
	local pattern = '([^'..delimiters..']+)'
	string.gsub(source, pattern, function(value) elements[#elements + 1] =     value;  end);
	return elements
 end

-- Expode a string into a table, each character as an entry
function util.explode(text)
  local result = {}
    
    for a = 1, string.len(text)+1 do  -- for 1 to the number of letters in our text + 1
      table.insert(result, string.sub(text, a,a)) -- split the string with the start and end at "a"
    end  -- repeat untill the for loop ends

  return result
end

function util.string_starts(String, Start)
   return string.sub(String,1,string.len(Start))==Start
end

function util.file_exists(name)
   local f = io.open(name,"r")
   if f ~= nil then io.close(f) return true else return false end
end

-- END OF MODULE
return util
