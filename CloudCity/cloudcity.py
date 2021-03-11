#!/usr/bin/env python3

# Copyright Jon Berg , turtlemeat.com
# Modified by nikomu @ code.google.com

import string,cgi,time
from os import curdir, sep
from http.server import BaseHTTPRequestHandler, HTTPServer, BaseHTTPRequestHandler, CGIHTTPRequestHandler
from urllib import parse
import re
import os # os. path

CWD = os.path.abspath('.')
## print CWD

PORT = 80
HOST = "0.0.0.0"
UPLOAD_PAGE = 'upload.html' # must contain a valid link with address and port of the server     s



# -----------------------------------------------------------------------

class MyHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        try:

            if self.path == '/' :
                self.send_response(200)
                self.send_header('Content-type',	'text/html')
                self.end_headers()
                self.wfile.write( ( "<form ENCTYPE=\"multipart/form-data\" method=\"post\">" ).encode() )
                self.wfile.write( ( "<input name=\"file\" type=\"file\"/>" ).encode() )
                self.wfile.write( ( "<input type=\"submit\" value=\"upload\"/></form>\n" ).encode() )
                return

            elif ( self.path.find( 'download' ) != -1):

                self.myparams = dict(parse.parse_qsl(parse.urlsplit(self.path).query))

                self.send_response(200)
                self.send_header('content-type', 'application/octet-stream')
                self.send_header('content-disposition', 'attachment;filename=' + self.myparams["file"] )
                self.send_header('content-length', os.path.getsize( "./files/" + self.myparams[ "file"]  ) )
                self.end_headers()

                f = open( "./files/" + self.myparams[ "file" ], 'rb' )
                self.wfile.write( f.read() )
                return

            elif ( self.path.find( 'onefilename' ) != -1):

                self.send_response(200)
                self.send_header('content-type', 'text/html')
                self.end_headers()

                self.entries = os.listdir('./files/')
                self.wfile.write( self.entries[0].encode() )
                return

            elif ( self.path.find( 'delete' ) != -1):

                self.myparams = dict(parse.parse_qsl(parse.urlsplit(self.path).query))

                self.send_response(200)
                self.end_headers()
                self.wfile.write( "removed".encode() )

                os.remove( "./files/" + self.myparams[ "file" ] )
                return

            else : # default: just send the file

                filepath = self.path[1:] # remove leading '/'

                f = open( "./files/" + os.path.join(CWD, filepath), 'rb' )

                self.send_response(200)
                self.send_header('Content-type',	'application/octet-stream')
                self.end_headers()
                self.wfile.write(f.read())
                f.close()
                return

            return # be sure not to fall into "except:" clause ?

        except IOError as e :
            # debug
            print( e )
            self.send_error(404,'File Not Found: %s' % self.path)


    def do_POST(self):

        # Create the files directory, if it doesn't already exist
        if not os.path.isdir("./files"):
            os.mkdir("./files")

        content_type = self.headers['content-type']
        if not content_type:
            print("Content-Type header doesn't contain boundary")
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
        path = "./files/"  # self.translate_path(self.path)
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

                self.send_response(200)
                self.end_headers()
                self.wfile.write( "Uploaded".encode() )

                return ("File '%s' upload success!" % fn)
            else:
                out.write(preline)
                preline = line
        return ("Unexpect Ends of data.")


def main():

    try:
        server = HTTPServer((HOST, PORT), MyHandler)
        print ('started httpserver...')
        server.serve_forever()
    except KeyboardInterrupt:
        print ('^C received, shutting down server')
        server.socket.close()

if __name__ == '__main__':
    main()
