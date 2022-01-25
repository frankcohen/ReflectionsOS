#!/usr/bin/env python3

import os
import sys

import bottle
from bottle import run, static_file
from bottle import template

from pathlib import Path

def resolve_path(path):
    if (sys.platform == 'win32'):
        return "\\" + path.replace('/', '\\')
    return path


app = bottle.Bottle()
serve_path = (len(sys.argv) > 1) and sys.argv[1] or os.getcwd()


HTML_HEADER = '''<html>
<head>
 <title>Starling Reflections Cloud City file services</title>
 <style>
  ul { padding: 0; list-style: none}
 li {
            list-style: none;
            padding: 0 .1em .1em .7em;
            margin: 0;
        }
a:link {
                color: #808080;
                text-decoration: none;
            }

            /* visited link */
            a:visited {
                color: #808080;
                text-decoration: none;
            }

            /* mouse over link */
            a:hover {
                color: #ff7d12;
            }

            /* selected link */
            a:active {
                color: #ff7d12;
            }
  .odd { background-color: white}
  .even { background-color: #f8f8f8}
 </style>
</head>
<body>
Starling Reflections Cloud City file services<br><br>
<a href="..">..</a><br>
<ul>'''


class FileServer:
    def __init__(self, hostname='0.0.0.0', port=8088):
        run(app, host=hostname, port=port)

    # Get JSON encoded list of files from /home/fcohen/files directory
    @app.route('/listfiles')
    def serve():
        path = '/home/fcohen/files' #os.path.join(serve_path, resolve_path('/home/fcohen/files'))
        filenum = 1;

        html = '''{'''
        flag1 = 0

        try:
            for fname in sorted(os.listdir(path) ):

                scheme = bottle.request.urlparts.scheme
                host = bottle.request.urlparts.netloc

                if flag1:
                    html = html + ''','''

                flag1 = 1;

                html = html + '''"'''
                html = html + str( filenum )
                filenum += 1
                html = html + '''":{"file": "'''
                html = html + fname
                html = html + '''","when": "'''
                html = html + str( os.path.getmtime( os.path.join(path, fname ) ) )
                html = html + '''"}'''

        except Exception as e:
            html = "Server Error:" + str(e)

        html = html + '''}'''
        return html

    @app.route('/touch/<filename:re:.*>')
    def serve(filename):
        path = os.path.join(serve_path + "/files", resolve_path(filename))
        if not os.path.isfile(path):
            return "Not a file " + path
        try:
            Path(path).touch()
        except Exception as e:
            return "Server Error:" + str(e)
        return "Touched"

    @app.route('/<filename:re:.*>')
    def serve(filename):
        alt = False  # for alternate row color
        path = os.path.join(serve_path, resolve_path(filename))
        html = HTML_HEADER

        if os.path.isfile(path):
            return static_file(resolve_path(filename), root=serve_path)  # serve a file
        else:
            try:
                for fname in sorted(os.listdir(path), key=lambda p: (not os.path.isdir(os.path.join(path, p)), p.casefold())):

                    scheme = bottle.request.urlparts.scheme
                    host = bottle.request.urlparts.netloc
                    css_class = alt and "odd" or "even"
                    alt = not alt
                    web_path = os.path.join(filename, fname)
                    abs_path = os.path.join(serve_path, filename, fname)
                    is_dir = os.path.isdir(abs_path)
                    icon = is_dir and '&#128193;' or '&#x1F4C3;'
                    html = html + f'<li class="{css_class}"><a href="{scheme}://{host}/{web_path}">{icon}&nbsp;{fname}</a></li>'
            except Exception as e:
                html = "Server Error:" + str(e)
        return html + """</ul></body></html>"""



if __name__ == '__main__':
    FileServer()
