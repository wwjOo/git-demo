# target:libmallochook.so

# libmallochook.so:mallochook.cpp
# 	g++ -std=c++17 -rdynamic -fPIC -shared -o libmallochook.so mallochook.cpp
	
# .PHONY: clean

# clean:
# 	rm -rf libmallochook.so

target:demo

demo:demo.cpp
	g++ -g demo.cpp -o demo -L/home/ubuntu/reactor/mallochook -lmallochook -rdynamic

#我们希望，只要输入”make clean“后，”rm *.o temp“命令就会执行。但是，当当前目录中存在一个和指定目标重名的文件时，例如clean文件，结果就不是我们想要的了。输入”make clean“后，“rm *.o temp” 命令一定不会被执行。
#解决的办法：将目标clean定义成伪目标就成了 无论当前目录下是否存在“clean”这个文件，输入“make clean”后，“rm *.o temp”命令都会被执行
.PHONY: clean

clean:
	rm -rf demo