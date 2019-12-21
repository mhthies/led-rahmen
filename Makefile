

build: LED-Rahmen.ino webdata.h
	arduino --verify LED-Rahmen.ino

upload: LED-Rahmen.ino webdata.h
	arduino --upload LED-Rahmen.ino

webdata.h: webdata/*
	xxd -i webdata/* > webdata.h
