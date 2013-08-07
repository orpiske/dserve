Download Server
============

This is a toy project and should not be taken seriously. 

This program is a remote download utility. It is composed of a download server, which receives the requests from the download client

Usage
----

First things first: you have to run the server

```
nohup ./dsserver&
```

If you don't use nohup, it will keep running in foreground. If you want help use:

```
./dsserver --help
```

```
Usage: 	-p	--port=<port> port used to listen to
	-h	--host=<ip/hostname> hostname/ip listen to to
	-q	--quiet runs quietly
	-d	--debug runs in debug mode
	-i	--init save options
	-h	--help show this help
```

Once you have the server running, you can add downloads with the following command line:

```
/dsclient --url http://my.server.com/file.txt
```

If you need help:


```
/dsclient --help
```

That will show you this:

```
Usage: 	-p	--port=<port> port used to connect to
	-h	--host=<ip/hostname> hostname/ip connect to
	-u	--url=<url> download url
	-s	--status download status
	-c	--cancel download
	-p	--pause download
	-r	--restart download
	-k	--keepalive don't disconnect from the server
	-i	--init save options
	-h	--help show this help
```

Default Options
----

You can set the default options in each of the respective user files. 

Server file: `~/.dsserver/dsserver.conf

```
server.listen.port=62434
server.listen.ip=0.0.0.0
server.connection.timeout=0
server.directory.log=/Users/otavio/.dsserver/logs/
server.directory.dest=/Users/otavio/Downloads
server.downloads.max=20
server.mode.quiet=0
server.mode.debug=1
```

Client file: `~/.dsclient/dsclient.conf`

```
client.server.port=62434
client.server.ip=127.0.0.1
client.connection.timeout=10
client.connection.keepalive=0
```

Compiling and Installing
----

It's pretty straightforward: you need [CMake](http://www.cmake.org/) and [Libcurl](http://curl.haxx.se/libcurl/). That's all.

```
ccmake . && make && make install
```

References
----

* [Main Site](http://orpiske.net/)
