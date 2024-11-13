
local cosem = {}
local util = require("util")

-- LOCAL VARIABLES
-----------------------------------------

-- PRIVATE FUNCTIONS (use 'local function ...'
-----------------------------------------


---------------------------------------------------------------------------------------------------------------------------
-- CLASS DEFINITION: "Value"
---------------------------------------------------------------------------------------------------------------------------
Value = {} -- Will remain empty as class
mt = {} -- Will contain everything the instances will contain _by default_

mt.new = function(self, cosem_type, data)
    local inst={}
    inst.cosem_type = cosem_type
    inst.data = data
    return setmetatable(inst,getmetatable(Value))
end

-- Transform a data from a string to a Lua table
-- Example of argument: "{ cosem_type=\"OCTETSTRING\", data=\"0;1;2;\" }"
mt.from_lua_string = function(self, str)
	local result = load("return"..str)()
	if type(result) ~= "table" then
		result = { cosem_type="ERROR" }
	end

    return setmetatable(result, getmetatable(Value))
end

-- level parameter is used to manage indendation of output. If nil, considered as 0
mt.to_string = function(self, level)
    local output = "";
    if level then
      level = level + 1
    else
      level = 0
    end

    local indent = "\r\n"..string.rep (" ", level*2)

    if self.cosem_type == "OBIS" then

      local class_id, ln, attribute = string.match(self.code, "(.*)-(.*)-(.*)")

      -- FIXME: parse the OBIS code and fill the class id and attribut id fields
      output = "//Logical Name = "..ln.."\r\n//Class_Id = "..class_id.."\r\n//Attribut_Id = "..attribute
      if type(self.data) == "table" then
        if #self.data >= 1 then
          output = output..self.data[1]:to_string(level)
        end
      end

    elseif (self.cosem_type == "STRUCTURE") or (self.cosem_type == "ARRAY") then
        -- Call the method for each structure element
        if type(self.data) == "table" then
            output = output..indent..self.size.." elements in the "..self.cosem_type
            for i=1, self.size do
              output = output..self.data[i]:to_string(level)
            end

        else
          print("Data is not a structure")
        end
 
    else
      output = indent.."element "..self.cosem_type..",  value:"..self.data
    end
    return output;

end

mt.append = function(self, value)
  local valid = false
  if (value ~= nil) then
    if (self.data == nil) then
      self.data = {}
    end
    table.insert(self.data, value);
    valid = true
  end
  return valid
end


mt.cosem_type = "Invalid" --standard foo
mt.__index = mt -- Look up all inexistent indices in the metatable

setmetatable(Value, mt)

---------------------------------------------------------------------------------------------------------------------------
-- PUBLIC FUNCTION DEFINITION: "cosem.parse_cayox_file"
---------------------------------------------------------------------------------------------------------------------------

-- Transform the Cayox Logical Name format into our own format
local function read_obis_code(lines)
  -- Example of line: //Logical Name =  0 0 148 12 0 255
  -- //Logical Name =  0 0 148 12 0 255
  -- //Class_Id =  5
  -- //Attribut_Id =  5

  local ln = "//Logical Name = "
  local class_id ="//Class_Id = "
  local attr = "//Attribut_Id = "

  local obis = ""
  local valid = false

  if util.string_starts(lines[1], ln) and util.string_starts(lines[2], class_id) and util.string_starts(lines[3], attr) then
    obis =  string.gsub(lines[2], class_id, "").."-"..string.gsub(lines[1], ln, "").."-"..string.gsub(lines[3], attr, "")
--    print("Found:"..obis)
    valid = true 
  end

  return valid, obis
end

local function is_data(line)
  -- Format examples:
  --        element UNSIGNED8,   value:47
  -- element OCTETSTRING,   value:0;1;
  local OctetString = "OCTETSTRING"
  local value = nil

  local data_type, value = string.match(line, "%s*element%s*(%w+),%s*value:(.*)")
  
  if data_type == OctetString then
--    print("Found:"..OctetString )
    value = Value:new(OctetString, value)

  elseif (data_type ~= nil) and (value ~= nil) then
 --     print("Found: "..data_type )
      value = Value:new(data_type, tonumber(value))
  else
    -- skip line
  end

  return value

end

local function is_array(line)

  local n = string.match(line, "%s*(%d+) elements in the array")
  return tonumber(n)
end

local function is_struct(line)

  local n = string.match(line, "%s*(%d+) elements in the structure")
  return tonumber(n)
end

function get_line(buffer)
  local line = nil
  if buffer.rd <= buffer.size then
    line = buffer.lines[buffer.rd]
    buffer.rd = buffer.rd + 1
  end
  return line
end


function Next(buffer, parent)
  local line  = get_line(buffer)

  if line then
    local array_size = is_array(line)
    local struct_size = is_struct(line)

    if array_size or struct_size  then
      local data_type = array_size and "ARRAY" or "STRUCTURE"
      local data_size = array_size and array_size or struct_size
      
   --   print("Found "..data_type.." of size: "..data_size)
      local object = Value:new(data_type, {})
      object.size = data_size

      -- Get the N next values of the structure or the array
      for i=1, data_size, 1 do
        Next(buffer, object)
      end
      parent:append(object)

    else
      -- Should be a data
      parent:append(is_data(line))
    end
  end
end


function cosem.parse_cayox_file(file)
  local valid = false
  local object = nil
  local obis = ""
  local buffer = { 
    size = 0,  -- number of lines
    rd = 0, -- read pointer
    lines = {} -- array representation of the file, each entry = one line
  }

  if util.file_exists(file) then
    -- First, copy lines into an array buffer

    for l in io.lines(file) do 
        table.insert(buffer.lines, l)
    end
    buffer.size = #buffer.lines
   
     -- At least 4 lines for every cayox file
     if (buffer.size >= 4) then
        valid, obis = read_obis_code(buffer.lines)
        buffer.rd = 4

        if (valid) then
          object = Value:new("OBIS")
          object.code = obis;

          Next(buffer, object)
        end
      end
   end
   return valid, object
end

function cosem.value(cosem_type, data)

  return Value:new(cosem_type, data)
  
end



-- END OF MODULE
return cosem

