#!/usr/bin/env python3

'''
 File upload/download services for the Reflections devices

 Reflections project: A wrist watch
 Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 player/recorder, SD card, GPS, and accelerometer/compass
 Repository and community discussions at https://github.com/frankcohen/ReflectionsOS

 Licensed under GPL v3
 (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 February 13, 2021

 Thank you Tao Huang of UniIsland for writing the code this is derived from:
 see: https://gist.github.com/UniIsland/3346170
 'This module builds on BaseHTTPServer by implementing the standard GET
 and HEAD requests in a fairly straightforward manner.'

 Available features:
 http://server-ip/download?file=fileone.jpeg gets a file from the server
 http://server-ip/ browses the file upload form
 http://server-ip/delete?file=fileone.jpeg delete a file
 http://server-ip/onefilename browses a single file on the server

 Notes on how I build and deploy this service:
 Uses Python's SimpleHTTPServer Module
   https://stackabuse.com/serving-files-with-pythons-simplehttpserver-module/
 Not for production use, there is no security to this service
 It's running on port 80 with no libraries.
 I threw this together quickly and would be glad for your improvements.

 AWS Lightsail Node.js and Mongo server
 https://docs.bitnami.com/aws/infrastructure/mean/
 https://docs.bitnami.com/aws/
 https://community.bitnami.com/

 Run the server:
 sudo python3 cloudcity.py

 stop it:
 ps -ef|grep python3
 sudo kill -9 process number
'''

__version__ = "1.0"
__all__ = ["CloudCity"]
__author__ = "fcohen@starlingwatch.com"
__home_page__ = "http://starlingwatch.com/"

import os
import posixpath
import http.server
import urllib.request, urllib.parse, urllib.error
import html
import shutil
import mimetypes
import re
from io import BytesIO
import cgi

hostName = "0.0.0.0"
serverPort = 80

class CloudCityServer(http.server.BaseHTTPRequestHandler):

    """Simple HTTP request handler with GET/HEAD/POST commands.

    This serves files from the current directory and any of its
    subdirectories.  The MIME type for files is determined by
    calling the .guess_type() method. And can reveive file uploaded
    by client.

    The GET/HEAD/POST requests are identical except that the HEAD
    request omits the actual contents of the file.

    """

    server_version = "SimpleHTTPWithUpload/" + __version__

    def do_GET(self):
        """Serve a GET request."""

        if self.path == '/onefilename':
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()

            self.wfile.write( os.listdir('/')[0] )

            f = self.send_head()
            if f:
                self.copyfile(f, self.wfile)
                f.close()

        elseif self.path == '/download':

            self.form = cgi.FieldStorage()
            self.filename = form.getvalue('file')

            f = self.send_head()
            if f:
                self.copyfile(f, self.wfile)
                f.close()

        elseif self.path == 'delete':
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            try:
                self.form = cgi.FieldStorage()
                self.filename = form.getvalue('file')
                os.remove( filename );
                self.wfile.write( "<html><body>Deleted</body></html>")
            except IOError:
                self.wfile.write( "<html><body>Could not delete ")
                self.wfile.write( filenname )
                self.wfile.write( "Error " )
                self.wfile.write( IOError )
                self.wfile.write( "</body></html>")

            f = self.send_head()
            if f:
                self.copyfile(f, self.wfile)
                f.close()

        else:
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write("<html><head><title>ReflectionsOS project</title></head>")
            self.wfile.write('<body>')
            self.wfile.write('<form action="fileupload" method="post" enctype="multipart/form-data">')
            self.wfile.write('<input type="file" name="filetoupload"><br>')
            self.wfile.write('<input type="submit">')
            self.wfile.write('<form>')

            f = self.send_head()
            if f:
                self.copyfile(f, self.wfile)
                f.close()

    def do_HEAD(self):
        """Serve a HEAD request."""
        f = self.send_head()
        if f:
            f.close()

    def do_POST(self):
        """Serve a POST request."""

        if self.path == '/fileupload':
            r, info = self.deal_post_data()
            print((r, info, "by: ", self.client_address))

            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.send_header("Content-Length", str(length))
            self.end_headers()
            if f:
                self.copyfile(f, self.wfile)
                f.close()

    def deal_post_data(self):
        content_type = self.headers['content-type']
        if not content_type:
            return (False, "Content-Type header doesn't contain boundary")
        boundary = content_type.split("=")[1].encode()
        remainbytes = int(self.headers['content-length'])
        line = self.rfile.readline()
        remainbytes -= len(line)
        if not boundary in line:
            return (False, "Content NOT begin with boundary")
        line = self.rfile.readline()
        remainbytes -= len(line)
        fn = re.findall(r'Content-Disposition.*name="file"; filename="(.*)"', line.decode())
        if not fn:
            return (False, "Can't find out file name...")
        path = self.translate_path(self.path)
        fn = os.path.join(path, fn[0])
        line = self.rfile.readline()
        remainbytes -= len(line)
        line = self.rfile.readline()
        remainbytes -= len(line)
        try:
            out = open(fn, 'wb')
        except IOError:
            return (False, "Can't create file to write, do you have permission to write?")

        preline = self.rfile.readline()
        remainbytes -= len(preline)
        while remainbytes > 0:
            line = self.rfile.readline()
            remainbytes -= len(line)
            if boundary in line:
                preline = preline[0:-1]
                if preline.endswith(b'\r'):
                    preline = preline[0:-1]
                out.write(preline)
                out.close()
                return (True, "File '%s' upload success!" % fn)
            else:
                out.write(preline)
                preline = line
        return (False, "Unexpect End of data.")

    def send_head(self):
        """Common code for GET and HEAD commands.

        This sends the response code and MIME headers.

        Return value is either a file object (which has to be copied
        to the outputfile by the caller unless the command was HEAD,
        and must be closed by the caller under all circumstances), or
        None, in which case the caller has nothing further to do.

        """
        path = self.translate_path(self.path)
        f = None
        if os.path.isdir(path):
            if not self.path.endswith('/'):
                # redirect browser - doing basically what apache does
                self.send_response(301)
                self.send_header("Location", self.path + "/")
                self.end_headers()
                return None
            for index in "index.html", "index.htm":
                index = os.path.join(path, index)
                if os.path.exists(index):
                    path = index
                    break
            else:
                return self.list_directory(path)
        ctype = self.guess_type(path)
        try:
            # Always read in binary mode. Opening files in text mode may cause
            # newline translations, making the actual size of the content
            # transmitted *less* than the content-length!
            f = open(path, 'rb')
        except IOError:
            self.send_error(404, "File not found")
            return None
        self.send_response(200)
        self.send_header("Content-type", ctype)
        fs = os.fstat(f.fileno())
        self.send_header("Content-Length", str(fs[6]))
        self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
        self.end_headers()
        return f

    def list_directory(self, path):
        """Helper to produce a directory listing (absent index.html).

        Return value is either a file object, or None (indicating an
        error).  In either case, the headers are sent, making the
        interface the same as for send_head().

        """
        try:
            list = os.listdir(path)
        except os.error:
            self.send_error(404, "No permission to list directory")
            return None
        list.sort(key=lambda a: a.lower())
        f = BytesIO()
        displaypath = html.escape(urllib.parse.unquote(self.path))
        f.write(b'<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">')
        f.write(("<html>\n<title>Directory listing for %s</title>\n" % displaypath).encode())
        f.write(("<body>\n<h2>Directory listing for %s</h2>\n" % displaypath).encode())
        f.write(b"<hr>\n")
        f.write(b"<form ENCTYPE=\"multipart/form-data\" method=\"post\">")
        f.write(b"<input name=\"file\" type=\"file\"/>")
        f.write(b"<input type=\"submit\" value=\"upload\"/></form>\n")
        f.write(b"<hr>\n<ul>\n")
        for name in list:
            fullname = os.path.join(path, name)
            displayname = linkname = name
            # Append / for directories or @ for symbolic links
            if os.path.isdir(fullname):
                displayname = name + "/"
                linkname = name + "/"
            if os.path.islink(fullname):
                displayname = name + "@"
                # Note: a link to a directory displays with @ and links with /
            f.write(('<li><a href="%s">%s</a>\n'
                    % (urllib.parse.quote(linkname), html.escape(displayname))).encode())
        f.write(b"</ul>\n<hr>\n</body>\n</html>\n")
        length = f.tell()
        f.seek(0)
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-Length", str(length))
        self.end_headers()
        return f

    def translate_path(self, path):
        """Translate a /-separated PATH to the local filename syntax.

        Components that mean special things to the local file system
        (e.g. drive or directory names) are ignored.  (XXX They should
        probably be diagnosed.)

        """
        # abandon query parameters
        path = path.split('?',1)[0]
        path = path.split('#',1)[0]
        path = posixpath.normpath(urllib.parse.unquote(path))
        words = path.split('/')
        words = [_f for _f in words if _f]
        path = os.getcwd()
        for word in words:
            drive, word = os.path.splitdrive(word)
            head, word = os.path.split(word)
            if word in (os.curdir, os.pardir): continue
            path = os.path.join(path, word)
        return path

    def copyfile(self, source, outputfile):
        """Copy all data between two file objects.

        The SOURCE argument is a file object open for reading
        (or anything with a read() method) and the DESTINATION
        argument is a file object open for writing (or
        anything with a write() method).

        The only reason for overriding this would be to change
        the block size or perhaps to replace newlines by CRLF
        -- note however that this the default server uses this
        to copy binary data as well.

        """
        shutil.copyfileobj(source, outputfile)

    def guess_type(self, path):
        """Guess the type of a file.

        Argument is a PATH (a filename).

        Return value is a string of the form type/subtype,
        usable for a MIME Content-type header.

        The default implementation looks the file's extension
        up in the table self.extensions_map, using application/octet-stream
        as a default; however it would be permissible (if
        slow) to look inside the data to make a better guess.

        """

        base, ext = posixpath.splitext(path)
        if ext in self.extensions_map:
            return self.extensions_map[ext]
        ext = ext.lower()
        if ext in self.extensions_map:
            return self.extensions_map[ext]
        else:
            return self.extensions_map['']

    if not mimetypes.inited:
        mimetypes.init() # try to read system mime.types
    extensions_map = mimetypes.types_map.copy()
    extensions_map.update({
        '': 'application/octet-stream', # Default
        '.py': 'text/plain',
        '.c': 'text/plain',
        '.h': 'text/plain',
        })

if __name__ == "__main__":
    cloudcity = CloudCityServer((hostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (hostName, serverPort))

    try:
        cloudcity.serve_forever()
    except KeyboardInterrupt:
        pass

    cloudcity.server_close()
    print("Server stopped.")
