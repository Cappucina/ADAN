set_project("ADAN")
set_version("1.0.0")

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
if is_plat("linux") or is_plat("macosx") then
	add_links("crypto")
end
set_rundir(".")
set_runargs("-f", "samples/testing.adn", "-o", "samples/testing")

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

	local function write_embedded_data(header_file, macro_name, content)
		local symbol_name = "LIBDATA_" .. macro_name
		header_file:write("static const unsigned char " .. symbol_name .. "[] = {\n")
		for i = 1, #content do
			if (i - 1) % 16 == 0 then
				header_file:write("    ")
			end
			header_file:write(string.format("0x%02X,", string.byte(content, i)))
			if i % 16 == 0 or i == #content then
				header_file:write("\n")
			else
				header_file:write(" ")
			end
		end
		header_file:write("    0x00\n};\n")
		header_file:write("#define LIB_" .. macro_name .. " ((const char*)" .. symbol_name .. ")\n\n")
	end

	local build_dir = "build"
	local header_path = path.join(os.projectdir(), build_dir, ".gens", target:name(), "embedded_libs_data.h")

	os.mkdir(path.directory(header_path))
	local header_file, err = io.open(header_path, "w")
	assert(header_file, "failed to open header file '" .. header_path .. "': " .. tostring(err))
	header_file:write("\n")

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
				write_embedded_data(header_file, macro_name, content)
			end
		end
	end

	header_file:write("#ifndef LIB_RUNTIME_ADN\n#define LIB_RUNTIME_ADN (void*)0\n#endif\n")
	header_file:close()
end)

task("format")
set_menu({ usage = "xmake format", description = "Format source code" })
on_run(function()
	local files = os.files("src/**.c")
	table.join2(files, os.files("src/**.h"))
	table.join2(files, os.files("libs/**.c"))
	table.join2(files, os.files("libs/**.h"))
	for _, file in ipairs(files) do
		os.exec("clang-format -i " .. file)
	end
	print("Format complete.")
end)

task("install")
set_menu({ usage = "xmake install", description = "Install dependencies" })
on_run(function()
	if is_host("windows") then
		os.exec("pwsh -ExecutionPolicy Bypass -File utils/dependencies.ps1")
	else
		os.exec("bash utils/dependencies.sh")
	end
end)
