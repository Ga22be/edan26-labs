.PHONY: all run clean transfer

all: classes

classes: dataflow.scala
	@mkdir -p classes
	#scalac -g:vars -d classes dataflow.scala
	#scalac -deprecation -d classes dataflow.scala
	scalac -deprecation -optimise -d classes dataflow.scala

run:
	./run.sh

clean:
	rm -rf classes

transfer:
	rsync -vrc . forsete.cs.lth.se:~/Documents/edan26/labs/lab2
