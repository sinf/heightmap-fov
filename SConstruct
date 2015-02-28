
src=Glob("code/*.c")

e=Environment()
e.VariantDir("build/debug","code",duplicate=0)
e.Append(CCFLAGS=Split("-std=c99 -Wall -g -O0 -Wno-missing-braces"))
e.Append(LIBS=Split("m freeimage"))
e.ParseConfig("pkg-config --libs --cflags gl glew")
e.ParseConfig("sdl-config --libs --cflags")
obj=e.Object(src)
bin=e.Program(target="fog",source=obj)
"""
e=Environment()
e.VariantDir("build/fast","code",duplicate=0)
e.Append(CCFLAGS=Split("-std=c99 -Wall -O3 -Wno-missing-braces"))
e.Append(LIBS=Split("m freeimage"))
e.ParseConfig("pkg-config --libs --cflags gl glew")
e.ParseConfig("sdl-config --libs --cflags")
obj=e.Object(src)
bin=e.Program(target="Fog",source=obj)
"""
