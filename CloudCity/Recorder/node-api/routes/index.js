var express = require('express');
var router = express.Router();
var path = require('path');
const folderPath = "/home/fcohen/files/";
// const folderPath = "/home/ec2-user/cloudCity/files/";
// "/var/www/node-api/uploads/final/";
const fs = require("fs");

/* GET home page. */
router.get('/', function (req, res, next) {
  res.render('index', { title: 'Express' });
});

/* file download through url */
router.get('/serve/*', (req, res) => {
  let url = req.url.split('/').pop();
  fs.readdir(folderPath, (err, files) => {
    files.forEach((file, index) => {
      if (file === url) {
        res.download(folderPath + url);
      }
    });
  });
});

/* list all files */
router.get('/files', function (req, res, next) {
  console.log(path.join(__dirname, "../index.html"));
  res.sendFile(path.join(__dirname, "../index.html"));  
});

/* list files in json */
router.get('/listfiles', function (req, res, next) {
  let name = {};
  fs.readdir(folderPath, (err, files) => {
    files.forEach((file, index) => {
      name[`${index + 1}`] = { file: file, size: fs.statSync(folderPath + file).size }
    });
    res.json(name);
  });
});

router.get('/filename', function (req, res, next) {
  let fileName = [];
  let data = fs.readdirSync(folderPath);  
  res.send({ data: data, folderPath: folderPath });
});

// endpoint for downloading a file
router.get("/download", (req, res) => {
  const filePath = folderPath + req.query.name;

  res.download(filePath, (err) => {
      if (err) {
          console.log(err);
      }
  });
});

// endpoint for deleting the files
router.get("/delete", (req, res) => {
  const filePath = folderPath + req.query.name;
  const fileNameWithoutExtension = path.parse(req.query.name).name; // gets filename without extension

  // Function to delete matching files from a directory
  const deleteMatchingFiles = (directoryPath) => {
      fs.readdir(directoryPath, (err, files) => {
          if (err) {
              console.log(err);
          } else {
              files.forEach(file => {
                  // If the file starts with the given name, delete it
                  if (file.startsWith(fileNameWithoutExtension)) {
                      fs.unlink(path.join(directoryPath, file), err => {
                          if (err) {
                              console.log(err);
                          } else {
                              console.log("File removed: " + path.join(directoryPath, file));
                          }
                      });
                  }
              });
          }
      });
  };

  // Delete the main file
  fs.unlink(filePath, function (err) {
      if (err) {
          console.log(err);
          res.status(404).send("File not found");
      } else {
          console.log("File removed: " + filePath);
          // Delete matching files in the other directories
          deleteMatchingFiles("../uploads/final");
          deleteMatchingFiles("../uploads/tmp");
          res.status(200).send("File deleted successfully");
      }
  });
});



/* touch file */
router.get('/touch/*', (req, res) => {
  let url = req.url.split('/').pop();
  fs.readdir(folderPath, (err, files) => {
    files.forEach((file, index) => {
      if (file === url) {
        const time = new Date();        
        try {
          fs.utimesSync(folderPath + url, time, time);
          res.send("Success")
        } catch (err) {
          fs.closeSync(fs.openSync(folderPath + url, 'w'));
        }
      }
    });
  });
});



module.exports = router;
