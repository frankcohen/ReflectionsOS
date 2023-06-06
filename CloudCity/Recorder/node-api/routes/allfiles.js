// app.get("/", (req, res) => {
//   console.log("tir");
//   res.sendFile(path.join(__dirname, "./index.html"));
// });

var express = require('express');
var router = express.Router();

/* GET allfiles page. */
router.get('/allfiles', function(req, res, next) {
 res.sendFile(path.join(__dirname, "../index.html"));
});

module.exports = router;
