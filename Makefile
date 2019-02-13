all:
	pandoc --template=./templates/GitHub.html5 index.md -o index.html