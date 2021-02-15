/**
 * File upload/download services for the Reflections devices
 *
 * Reflections project: A wrist watch
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Repository and community discussions at https://github.com/frankcohen/ReflectionsOS
 * Licensed under GPL v3
 * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 * February 13, 2021

 Available features:
 http://server-ip/download?file=fileone.jpeg gets a file from the server
 http://server-ip/ browses the file upload form
 http://server-ip/delete?file=fileone.jpeg delete a file
 http://server-ip/onefilename browses a single file on the server

 Thanks to Dagm Fekadu for showing me how to do binary attachment downloads in node
 https://stackoverflow.com/questions/33052252/setting-file-name-for-download-response-in-node-js

 Notes on how I build and deploy this service:

 I stay away from express and node.js project structures to keep this really simple.
 It's running on port 80 with minimal libraries.
 I threw this together quickly and would be glad for your improvements.

 AWS Lightsail Node.js and Mongo server
 https://docs.bitnami.com/aws/infrastructure/mean/
 https://docs.bitnami.com/aws/
 https://community.bitnami.com/

 Run the node.js project
 cd /opt/bitnami/projects/
 sudo node reflections.js&
 stop it:
 ps aux|grep node
 sudo kill -9 process number

 Requires ‘formidable’ package: npm install formidable
 and npm install download-file --save

 Service status
 sudo /opt/bitnami/ctlscript.sh status
 Start all services
 sudo /opt/bitnami/ctlscript.sh start
 Restart
 sudo /opt/bitnami/ctlscript.sh restart apache

 Node.js file upload utility:
 https://www.w3schools.com/nodejs/nodejs_uploadfiles.asp

*/

var http = require('http');
var formidable = require('formidable');
var fs = require('fs');
const { exec } = require("child_process");
const url = require('url');
const pathToFiles = '/opt/bitnami/projects/reflections/files/';

http.createServer(function(req, res) {

  const feature = url.parse(req.url, true).pathname;

  if (feature == '/fileupload') {
    var form = new formidable.IncomingForm();
    form.parse(req, function(err, fields, files) {
	    
      var oldpath = files.filetoupload.path;
      var newpath = pathToFiles + files.filetoupload.name;
      
      exec("xxd -e -g2 " + oldpath + " | xxd -r > " + newpath );  
      res.write('File uploaded and moved.');
      res.end();
    });
  } else if (feature == '/onefilename') {

		// Called with http://server-ip/onefilename to browse a single file on the server

    const baseUrl = pathToFiles;
    fs.readdir(baseUrl, (err, files) => {
      if (err || files.length==0)
      {
        res.end( "nofiles" );
      }
      else
      {
        res.write(files[0]);
	res.end();
      }

    });

  } else if (feature == '/download') {

		// Called with http://server-ip/download?file=fileone.jpeg to get a file from the server

    const fileName = url.parse(req.url, true).query.file;
    const directoryPath = pathToFiles;
    const myname = directoryPath + fileName;

    fs.exists(myname, function(exists) {
      if (!exists) {
        res.writeHead(404, {
          "Content-Type": "text/plain"
        });
        res.write("File Not found: 404 Not Found\n");
        res.end(myname);
        return;
      }

      fs.readFile(myname, "binary", function(err, file) {
        if (err) {
          res.writeHead(500, {
            "Content-Type": "binary"
          });
          res.write(err + "\n");
          res.end();
          return;
        }

        res.writeHead(200, {
          "content-disposition": "attachment;filename=" + fileName,
          'content-type': 'application/octet-stream',
          'content-length': file.length
        });

        res.write(file,"binary");
        res.end();
      });
    });

  } else if (feature == '/delete') {

		// called with http://server-ip/delete?file=fileone.jpeg to delete a file

    try {
      const fileName = url.parse(req.url, true).query.file;
      const directoryPath = pathToFiles;

      fs.unlinkSync(directoryPath + fileName)
      res.end("deleted");
    } catch (err) {
      res.end(err);
    }

  } else {

		// Called with http://server-ip/ to browse the file upload form

    res.writeHead(200, {
      'Content-Type': 'text/html'
    });
    res.write('<form action="fileupload" method="post" enctype="multipart/form-data">');
    res.write('<input type="file" name="filetoupload"><br>');
    res.write('<input type="submit">');
    res.write('</form>');
    return res.end();
  }
}).listen(80);
