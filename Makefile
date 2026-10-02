.PHONY: all clean
	
DATE = $(shell date +"%Y-%m-%d %H:%M:%S")

SRCS = $(wildcard *.md)
HTML = $(SRCS:.md=.html)
GITHUB = https://github.com/diegocaro/diegocaro.github.io/blob/main

FLAGS = -V date-meta="${DATE}" -V file-meta="${GITHUB}/$<" --template=./templates/mypage.html5

SHTMLFILES := $(shell find . -type f -name "*.shtml")
HTMLFILES := $(SHTMLFILES:.shtml=.html)

all: template ${HTML} ${HTMLFILES}

%.html: %.md
	pandoc ${FLAGS} $< -o $@
	
template: ./templates/mypage.html5
	
%.html: %.shtml
	./scripts/ssi.py $< $@ 	

# clean:
# 	-rm -f ${HTML}