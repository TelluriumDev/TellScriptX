add_rules("mode.debug", "mode.release")

add_requires("quickjs 2024.01.13", {configs={shared=true}})
add_requires("lua", {configs={shared=true}})

option("backend")
    set_default("quickjs")
    set_values("quickjs", "lua")

target("ScriptX")
    set_languages("cxx20")
    add_files("src/**.cc")
    add_headerfiles("(**.h)","(**.hpp)")
    add_includedirs("src/include/")
    set_toolchains("mingw")

    set_kind("static")   

if is_config("backend", "quickjs") then
    add_packages(
            "quickjs"
        )
    add_defines(
            "SCRIPTX_BACKEND_QUICKJS",
            "SCRIPTX_BACKEND_TRAIT_PREFIX=../backend/QuickJs/trait/Trait"
        )
    add_files(
            "backend/QuickJs/**.cc"
        )
        
elseif is_config("backend", "Lua") then
    add_packages(
            "lua"
        )
    add_defines(
            "SCRIPTX_BACKEND_LUA",
            "SCRIPTX_BACKEND_TRAIT_PREFIX=../backend/Lua/trait/Trait"
        )
    add_files(
            "backend/Lua/**.cc"
        )
end
