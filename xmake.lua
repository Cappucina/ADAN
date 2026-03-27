set_project("ADAN")
set_version("1.0.0")

import("core.base.try")

set_languages("c11")
add_defines("_GNU_SOURCE")

if is_plat("windows") then
	add_defines("strtok_r=strtok_s", "strdup=_strdup")
end

target("adan_linker")
set_kind("static")
add_files("src/backend/linker/linker.c")
add_includedirs("src", "src/backend/linker", { public = true })

target("adan")
set_kind("binary")
add_files("src/**.c|backend/linker/linker.c")
add_files("libs/**.c")
add_deps("adan_linker")
add_includedirs("src", "libs")
add_includedirs("build/.gens/adan", { public = true })

before_build(function(target)
	local function generate_macro_name(rel_path)
		local parts = rel_path:split("[/\\]")
		local macro_name = ""
		if #parts > 1 then
			local dir_name = parts[1]
			local file_parts = {}
			for i = 2, #parts do
				table.insert(file_parts, parts[i])
			end
			local file_part = table.concat(file_parts, "_")
			local file_id = file_part:gsub("%.", "_")
			local dir_up = dir_name:upper()
			local file_up = file_id:upper()
			if file_up:find("^" .. dir_up .. "_") then
				macro_name = file_up
			else
				macro_name = dir_up .. "_" .. file_up
			end
		else
			macro_name = rel_path:gsub("%.", "_"):upper()
		end
		return macro_name
	end

	local build_dir = "build"
	local header_path = path.join(os.projectdir(), build_dir, ".gens", target:name(), "embedded_libs_data.h")

	os.mkdir(path.directory(header_path))
	local header_file, err = io.open(header_path, "w")
	assert(header_file, "failed to open header file '" .. header_path .. "': " .. tostring(err))
	header_file:write("// Auto-generated embedded libs\n\n")

	local embed_dirs = {
		{ dir = path.join(os.projectdir(), "libs"), prefix = "libs" },
		{ dir = path.join(os.projectdir(), "src/backend/runtime"), prefix = "runtime" },
	}

	for _, item in ipairs(embed_dirs) do
		local dir = item.dir
		if os.exists(dir) then
			local files = os.files(path.join(dir, "**.adn"))
			table.join2(files, os.files(path.join(dir, "**.c")))
			table.join2(files, os.files(path.join(dir, "**.h")))

			for _, file in ipairs(files) do
				local rel_path = path.relative(file, dir)
				local macro_name = generate_macro_name(rel_path)

				local content = io.readfile(file)
				if not content then
					raise("failed to read file: %s", file)
				end
				-- Escape content for inclusion as a C string literal in a header:
				-- 1) escape backslashes and double quotes
				-- 2) escape control characters (including null bytes) using C-style escapes
				-- 3) convert remaining newlines into '\n' and optionally split the C string
				--    across multiple lines in the generated source for readability.
				-- Step 1: escape backslashes and double quotes.
				content = content:gsub("\\", "\\\\")
				content = content:gsub('"', '\\"')
				-- Step 2: escape control characters (ASCII < 32 and null).
				content = content:gsub("[%z\1-\31]", function(c)
					local byte = string.byte(c)
					if byte == 10 then -- '\n'
						return "\\n"
					elseif byte == 13 then -- '\r'
						return "\\r"
					elseif byte == 9 then -- '\t'
						return "\\t"
					elseif byte == 8 then -- '\b'
						return "\\b"
					elseif byte == 11 then -- '\v'
						return "\\v"
					elseif byte == 12 then -- '\f'
						return "\\f"
					elseif byte == 7 then -- '\a'
						return "\\a"
					elseif byte == 0 then -- '\0'
						return "\\0"
					else
						return string.format("\\x%02X", byte)
					end
				end)
				-- Step 3: split the C string across multiple source lines for readability.
				-- At this point, actual newlines in 'content' should be rare; if any remain,
				-- replace them with an escaped '\n' and start a new C string literal.
				content = content:gsub("\n", '\\n"\n"')
				header_file:write("#define LIB_" .. macro_name .. ' "' .. content .. '"\n\n')
			end
		end
	end

	header_file:write("#ifndef LIB_RUNTIME_ADN\n#define LIB_RUNTIME_ADN (void*)0\n#endif\n")
	header_file:close()
end)

local function run_sample_test(targetfile)
	os.runv(targetfile, { "-f", "samples/testing.adn", "-o", "samples/testing" })
	local sample_exe = is_host("windows") and "samples/testing.exe" or "samples/testing"
	os.exec(path.join(".", sample_exe))
	os.rm("samples/testing.ll")
	os.rm(sample_exe)
end

on_run(function(target)
	local targetfile = target:targetfile()
	try
	{
		function ()
			run_sample_test(targetfile)
		end
	}
	catch
	{
		function (errors)
			-- stop further processing if the sample build or run fails
			raise(errors)
		end
	}
end)

after_build(function(target)
	import("core.base.option")
	if option.get("task") == nil then
		local targetfile = target:targetfile()
		try
		{
			function ()
				run_sample_test(targetfile)
			end
		}
		catch
		{
			function (errors)
				-- stop further processing if the sample build or run fails
				raise(errors)
			end
		}
	end
end)

task("format")
set_menu({ usage = "xmake format", description = "Format source code" })
on_run(function()
	local files = os.files("src/**.c")
	table.join2(files, os.files("src/**.h"))
	table.join2(files, os.files("libs/**.c"))
	table.join2(files, os.files("libs/**.h"))
	for _, file in ipairs(files) do
		os.execv("clang-format", { "-i", file })
	end
	print("Format complete.")
end)

task("install")
set_menu({ usage = "xmake install", description = "Install dependencies" })
on_run(function()
	if is_host("windows") then
		os.execv("pwsh", { "-ExecutionPolicy", "Bypass", "-File", "utils/dependencies.ps1" })
	else
		os.execv("bash", { "utils/dependencies.sh" })
	end
end)
