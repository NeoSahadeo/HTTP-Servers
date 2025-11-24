# HTTP 0.9

HTTP 0.9 is an informal standard to retrieve information from the server. That
information being HTML

## Usage

After cloning the repo, run the make file.

```bash
make all
```

The server can be started with, it optionally takes in a port number.

```bash
./server
```

Then to send requests to the client you can you use the client executable.

```bash
./client PORT MESSAGE
```

__EXAMPLE__

```bash
./client 8080 "GET user/dummy.html"
```

## Spec

<h6>
	Connection
</h6>
<ol>
	<li data-list-item-id="ee1d06772751bc742f1b8808f1f38283c">
		Create a tcp-ip connection, port 80 is the default
	</li>
	<li data-list-item-id="eecf09bfb43d1adbe004deb55341d93e5">
		The server then excepts the connection
	</li>
</ol>
<h6>
	Request
</h6>
<ol>
	<li data-list-item-id="e473d3b6559da630e716fea22f3bcb90e">
		Send a line of ASCII characters termined with CR LF
	</li>
	<li data-list-item-id="e9873a4ee33162b2fb05c826fe5ca5af8">
		The line should have “GET” + a space + address of the document
	</li>
</ol>
<h6>
	Response
</h6>
<ol>
	<li data-list-item-id="ee008b525c8a152afa700e328db744862">
		The response should be message in HTML
	</li>
	<li data-list-item-id="e5fdd0047f64fce16eeb693c4dc09706b">
		The line should be terminated with a line feed, optionally with a carriage return
	</li>
	<li data-list-item-id="e930ea30ba04e309cb59765bf4b117ebc">
		Line length restriction of 80 chars excluding CR LF
	</li>
	<li data-list-item-id="ec1560926cbd0035c6251819dc81e3eee">
		The format of the message is HTML, trimmed SGML.
	</li>
	<li data-list-item-id="e17b0bd13ba0a4e32f692edfbc6c3f5c5">
		Closes connection
	</li>
	<li data-list-item-id="e764f359024413b1f65467ec28a5ec794">
		Clients should read the entire document, optionally 15sec to close on inactivity
	</li>
	<li data-list-item-id="ec66e819ad8c463b8d1b9a531554ded68">
		Errors are sent in human readable text in HTML syntax
	</li>
</ol>
<h6>
	Disconnection
</h6>
<ol>
	<li data-list-item-id="e89dc2ef2d1ad07588fc58e40a62b6d0b">
		After the while document is transferred the connection is closed by the server.
	</li>
	<li data-list-item-id="e873a601a761a310ef93efa998f6edc1b">
		The client may break the connection
	</li>
	<li data-list-item-id="e0ee5ff470da13b39f1073bad789cdf15">
		No info for the request needs to be stored; idempotent.
	</li>
</ol>
<p>
	<a href="https://www.w3.org/Protocols/HTTP/AsImplemented.html">
		https://www.w3.org/Protocols/HTTP/AsImplemented.html
	</a>
</p>

## License

MIT License 2025 Neo Sahadeo
