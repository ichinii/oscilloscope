conf = ScriptArgs["conf"] or "debug"
build_dir = ScriptArgs["build_dir"] or "build"
build_dir = PathJoin(build_dir, conf)
src_dir = "src"
bin_name = ScriptArgs["name"] or "oscilloscope"

AddTool(function(settings)
	if conf == "debug" then
		settings.debug = 1
		settings.optimize = 0
	  settings.cc.flags:Add("-g")
	elseif conf == "release" then
		settings.debug = 0
		settings.optimize = 1
	end

	settings.cc.flags:Add("-Wall")
	settings.cc.flags:Add("-Wextra")
	settings.cc.flags_cxx:Add("--std=c++17")
	settings.cc.includes:Add("/usr/include")
	settings.cc.includes:Add(src_dir)
	settings.link.libs:Add("GL")
	settings.link.libs:Add("GLU")
	settings.link.libs:Add("GLEW")
	settings.link.libs:Add("glfw")

	settings.cc.Output = function(settings, input)
		input = input:gsub("^"..src_dir.."/", "")
		return PathJoin(PathJoin(build_dir, "obj"), PathBase(input))
	end
	settings.link.Output = function(settings, input)
		return PathJoin(build_dir, PathBase(input))
	end

	return settings
end)

settings = NewSettings()

src = CollectRecursive(PathJoin(src_dir, "*.cpp"))
obj = Compile(settings, src)
bin = Link(settings, bin_name, obj)

DefaultTarget(bin)
PseudoTarget("run")
AddDependency("run", bin)
AddJob("run", "running './"..bin.."'...", "./"..bin)
