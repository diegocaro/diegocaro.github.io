.PHONY: all clean
	
DATE = $(shell date +"%Y-%m-%d %H:%M:%S")

SRCS = $(wildcard *.md)
HTML = $(SRCS:.md=.html)
GITHUB = https://github.com/diegocaro/diegocaro.github.io/blob/main

FLAGS = -V date-meta="${DATE}" -V file-meta="${GITHUB}/$<" --template=./templates/mypage.html5

all: template ${HTML} 

%.html: %.md
	pandoc ${FLAGS} $< -o $@
	
template: ./templates/mypage.html5
	
	

clean:
	-rm -f ${HTML}