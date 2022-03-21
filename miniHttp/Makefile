bin = httpserver
cc = g++
cgi = test_cgi
LD_FLAGS = -std=c++11 -lpthread
src = main.cc
curr := $(shell pwd) #获取当前所在路径

ALL::$(bin) $(cgi)
.PHONY:ALL 


$(bin):$(src)
	$(cc) -o $@ $^ $(LD_FLAGS)

$(cgi):cgi/test_cgi.cc 
	$(cc) -o $@ $^

.PHONY:clean
clean:
	rm $(bin) $(cgi)
	rm -r output

.PHONY:output
output:
	mkdir -p output 
	cp $(bin) output 
	cp -r wwwroot output
	cp $(cgi) output/wwwroot 
	cp ./cgi/python_cgi.py output/wwwroot 
	cp ./cgi/shell_cgi.sh output/wwwroot
