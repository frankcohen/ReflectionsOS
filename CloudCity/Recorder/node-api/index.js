var express = require('express');
const path = require('path');
var cors = require('cors');
var app = express();

app.set('view engine', 'pug');
app.set('views', path.join(__dirname, 'views'));

// Allow origins from https://cloudcity.starlingwatch.com/
app.use(cors({
    origin: 'https://cloudcity.starlingwatch.com'
}));

var indexRouter = require('./routes/index');
var usersRouter = require('./routes/users');
var allFilesRouter = require('./routes/allfiles');

app.use('/api', indexRouter);
app.use('/api/users', usersRouter);
app.use('/api/allfiles', allFilesRouter);

app.listen(3000, '0.0.0.0', function () {
    console.log('Server is running on http://localhost:3000');
});
