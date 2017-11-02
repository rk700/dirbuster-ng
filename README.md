dirbuster-ng
------------

dirbuster-ng is a light-weight and fast tool for brute-forcing web paths.

## Build

run the following commands in the root source directory:

```bash
$ cmake .
$ make
$ make install
```

## Usage

```bash
$ dirbuster-ng -h
Usage: dirbuster-ng [options...] <url>
    Options:
    -w <nb_threads>     Defines the number of threads to use to make the attack(4 by default)
    -U <user_agent>     Defines a custom user-agent
    -A <username:password>      Sets username and password for HTTP auth
    -X <proxy_server:port>      Use an HTTP proxy server to perform the queries
    -K <username:password>      Sets an username/password couple for proxy auth
    -d <dict>   Loads an external textfile to use as a dictionary
    -e <ext>    Specify a list of extensions that would be appended to each word in the dict; seperated by comma; omitting this option would lead to dict entries unchanged
    -t <seconds>        Sets the timeout in seconds for each http query(10 by default)
    -W <file>   Saves the program's result inside a file
    -u <ua>     use a predefined user-agent, corresponding to the most used browsers/crawlers:
                ff: Mozilla Firefox
                chr: Google Chrome
                saf: Apple Safari
                ope: Opera
                opem: Opera Mobile
                ie: Microsoft Internet Explorer
                fen: Mozilla Fennec
                bb: RIM BlackBerry Browser
                gbot: Google Bot
                bing: Microsoft Bing Crawler
                bspid: Baidu Spider
    -q  Enable quiet mode (relevent only with the -W flag)
    -h  Prints this help then exits
    -V  Verbose. Print each request url and response code
    -v  Prints the program version then exits
    -f  Follow redirects(302 or 301)
```

## Example

```bash
$ cat dict
www
wwwroot
1
2
3
123
a
b
c
abc
backup

$ dirbuster-ng -d dict -e '.rar,.zip,.tar.gz,.7z,.gzip' -V http://127.0.0.1/
{Dirbuster NG 0.2} (c)2017 WintermeW, Ruikai Liu
[404] http://127.0.0.1/www.zip
[404] http://127.0.0.1/www.tar.gz
[404] http://127.0.0.1/www.7z
[404] http://127.0.0.1/www.rar
[404] http://127.0.0.1/wwwroot.rar
[404] http://127.0.0.1/www.gzip
[404] http://127.0.0.1/wwwroot.zip
[404] http://127.0.0.1/wwwroot.tar.gz
[404] http://127.0.0.1/1.rar
[404] http://127.0.0.1/wwwroot.7z
[404] http://127.0.0.1/1.zip
[404] http://127.0.0.1/wwwroot.gzip
[404] http://127.0.0.1/1.tar.gz
...

```
