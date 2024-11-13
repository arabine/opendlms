-- Modified by Anthony Rabine

local description = [=[
Usage: lua bin2c.lua [+]filename
Write a C source file to standard output.
Original author: Mark Edgar (MIT license)
]=]

if not arg or not arg[1] then
  io.stderr:write(description)
  return -1
end

local function get_filename(file)
	local path, name, ext = file:match("(.-)([^\\/]-%.?([^%.\\/]*))$")
	return name;
end

local function get_c_name(file)
	return string.gsub(get_filename(file), "%.", '_')
end

local function generate(file)

	local filename = get_c_name(file)

	local header = "\nstatic const std::uint8_t "..filename.."[] = { \n"
	local footer = "\n};\n\n"

	-- Read the whole file and store it into a variable
	local content = assert(io.open(file,"rb")):read"*a"

	local dump do
	  local numtab={}; for i=0,255 do numtab[string.char(i)]=("%3dU,"):format(i) end
	  function dump(str)
		return (str:gsub(".", numtab):gsub(("."):rep(80), "%0\n"))
	  end
	end

	io.write(header)
	io.write(dump(content))
	io.write(footer)
	
	return content:len()
end

local top = "#include \"Efs.h\"\n"
local middle = "static const Efs cList[cNumberOfFiles] = {\n"
local bottom = [=[
bool GetFile(const std::string &name, Efs &efs)
{
	bool found = false;
	for (std::uint32_t i = 0U; i < cNumberOfFiles; i++)
	{
		std::string file(cList[i].name);
		if (file == name)
		{
			found = true;
			efs = cList[i];
		}
	}
	return found;
}
]=]

io.write(top)
for i=1, #arg, 1 do
	local size = generate(arg[i])
	middle = middle.."\t{ \""..get_filename(arg[i]).."\", &"..get_c_name(arg[i]).."[0], "..size.."U }, \n"
end
io.write("static const std::uint32_t cNumberOfFiles = "..#arg.."U;\n")
io.write(middle.."};\n\n")
io.write(bottom)
