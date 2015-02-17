e=Environment()
e.Append(CCFLAGS=Split("-std=c99 -Wall -g -O0"))
e.Append(LIBS=Split("m freeimage"))
e.ParseConfig("pkg-config --libs --cflags gl glew")
e.ParseConfig("sdl-config --libs --cflags")
src=Glob("*.c")
obj=e.Object(src)
bin=e.Program(target="fog",source=obj)

