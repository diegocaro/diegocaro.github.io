.PHONY: all clean
	
DATE = $(shell date +%Y-%m-%d%n)

SRCS = $(wildcard *.md)
HTML = $(SRCS:.md=.html)

FLAGS = -V date-meta=${DATE} --template=./templates/mypage.html5

all: ${HTML}

%.html: %.md
	pandoc ${FLAGS} $< -o $@
	
clean:
	-rm -f ${HTML}