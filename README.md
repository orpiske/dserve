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

References
----

* [Main Site](http://orpiske.net/)
