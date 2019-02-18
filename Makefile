NOW = $(shell date +%Y-%m-%d%n)

all:
	pandoc --template=./templates/mypage.html5 index.md -o index.html -V date-meta=${NOW}