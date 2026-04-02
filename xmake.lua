set_project("ADAN")
set_version("1.0.0")

set_languages("c11")
add_defines("_GNU_SOURCE")

if is_plat("windows") then
	add_defines("strtok_r=strtok_s", "strdup=_strdup")
end

local function table_contains(list, value)
	for _, item in ipairs(list) do
		if item == value then
			return true
		end
	end
	return false
end

local function push_unique_path(list, value)
	if value and value ~= "" and os.isdir(value) and not table_contains(list, value) then
		table.insert(list, value)
	end
end

local function detect_openssl_config()
	local include_dirs = {}
	local link_dirs = {}
	local libs = {}

	local function add_root(root)
		if not root or root == "" then
			return
		end

		push_unique_path(include_dirs, path.join(root, "include"))

		local lib_candidates = {
			path.join(root, "lib"),
			path.join(root, "lib64"),
		}

		if is_plat("windows") then
			table.insert(lib_candidates, path.join(root, "lib", "VC", "x64", "MD"))
			table.insert(lib_candidates, path.join(root, "lib", "VC", "x64", "MT"))
			table.insert(lib_candidates, path.join(root, "lib", "VC", "x86", "MD"))
			table.insert(lib_candidates, path.join(root, "lib", "VC", "x86", "MT"))
			table.insert(lib_candidates, path.join(root, "lib", "VC", "static"))
			table.insert(lib_candidates, path.join(root, "lib", "MinGW"))
		end

		for _, candidate in ipairs(lib_candidates) do
			push_unique_path(link_dirs, candidate)
		end
	end

	add_root(os.getenv("OPENSSL_ROOT_DIR"))
	add_root(os.getenv("OPENSSL_DIR"))
	push_unique_path(include_dirs, os.getenv("OPENSSL_INCLUDE_DIR"))
	push_unique_path(link_dirs, os.getenv("OPENSSL_LIB_DIR"))

	if is_plat("macosx") then
		local brew_prefix = os.getenv("HOMEBREW_PREFIX")
		if brew_prefix and brew_prefix ~= "" then
			add_root(path.join(brew_prefix, "opt", "openssl@3"))
			add_root(path.join(brew_prefix, "opt", "openssl"))
		end

		add_root("/opt/homebrew/opt/openssl@3")
		add_root("/opt/homebrew/opt/openssl")
		add_root("/usr/local/opt/openssl@3")
		add_root("/usr/local/opt/openssl")
		add_root("/opt/local/libexec/openssl3")
		add_root("/opt/local/libexec/openssl11")
	elseif is_plat("windows") then
		local program_files = os.getenv("ProgramFiles")
		local program_files_x86 = os.getenv("ProgramFiles(x86)")
		local program_w6432 = os.getenv("ProgramW6432")
		local userprofile = os.getenv("USERPROFILE")

		add_root("C:/OpenSSL-Win64")
		add_root("C:/OpenSSL-Win32")

		if program_files and program_files ~= "" then
			add_root(path.join(program_files, "OpenSSL-Win64"))
			add_root(path.join(program_files, "OpenSSL-Win32"))
		end
		if program_files_x86 and program_files_x86 ~= "" then
			add_root(path.join(program_files_x86, "OpenSSL-Win32"))
		end
		if program_w6432 and program_w6432 ~= "" then
			add_root(path.join(program_w6432, "OpenSSL-Win64"))
		end
		if userprofile and userprofile ~= "" then
			add_root(path.join(userprofile, "scoop", "apps", "openssl", "current"))
		end
	end

	if is_plat("windows") then
		table.insert(libs, "libcrypto")
	else
		table.insert(libs, "crypto")
	end

	return {
		include_dirs = include_dirs,
		link_dirs = link_dirs,
		libs = libs,
	}
end

local openssl_config = detect_openssl_config()

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
if #openssl_config.include_dirs > 0 then
	add_includedirs(table.unpack(openssl_config.include_dirs))
end
if #openssl_config.link_dirs > 0 then
	add_linkdirs(table.unpack(openssl_config.link_dirs))
end
if #openssl_config.libs > 0 then
	add_links(table.unpack(openssl_config.libs))
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
