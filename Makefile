.PHONY: all clean
	
DATE = $(shell date +"%Y-%m-%d %H:%M:%S")

SRCS = $(wildcard *.md)
HTML = $(SRCS:.md=.html)

FLAGS = -V date-meta="${DATE}" -V file-meta="$<" --template=./templates/mypage.html5

all: template ${HTML} 

%.html: %.md
	pandoc ${FLAGS} $< -o $@
	
template: ./templates/mypage.html5
	
	

clean:
	-rm -f ${HTML}