 
/**
  wtonode.org/really-simple-file-uploads‎
  이 페이지 번역하기
  작성자: srirangan@gmail.com (Srirangan)
  2012. 2. 5. - It was the first time I was implementing file uploads with Node (and ... So I did the next most obvious thing - post a question on the mailing list.
  File uploads using Node.js: now for real | Componentix blog
 * Module dependencies.
 */

var express = require('express');
var routes = require('./routes');
var http = require('http');
var path = require('path');
var fs = require('fs');
var url = require('url');
var ejs = require('ejs');

var userInfo = require('./routes/userinfo');
var fileInfo = require('./routes/fileinfo');
var app = express();

// all environments
app.set('port', process.env.PORT || 3000);
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'ejs');
app.use(express.favicon());
app.use(express.logger('dev'));
app.use(express.json());
app.use(express.multipart());
app.use(express.urlencoded());
app.use(express.methodOverride());
app.use(express.cookieParser('your secret here'));
app.use(express.session());
app.use(express.bodyParser({ keepExtensions: true, uploadDir: '/uploads'}));
app.use(app.router);


//web
app.get('/upload/:deviceID', function(req, res) {
	var devID = req.params.deviceID;
	var query = url.parse(req.url, true).query;
	query = JSON.stringify(query);
	console.log("query : " + query);
	var str = JSON.parse(query);
	console.log("str.path : " + str.path);
	var path = str.path;

	fs.readFile(__dirname + '/html/upload.html', 'utf8', function(error, data) {
		res.send(ejs.render(data, {devID : devID, path : path}));
	});
});

app.get('/', function(req, res){
		fs.readFile(__dirname + '/html/login.html', function(error, data){
			res.writeHead(200, { 'Content-Type' : 'text/html' });
			res.end(data, function(error){
				console.log(error);
			});
		});
});

app.get('/enterAuthCode', function(req, res){
		fs.readFile(__dirname + '/html/authCode.html', function(error, data){
			res.writeHead(200, { 'Content-Type' : 'text/html' });
			res.end(data, function(error){
				console.log(error);
			});
		});
});

app.get('/mkdirForWeb/:deviceID', function(req, res){
	
	var devID = req.params.deviceID;
	var query = url.parse(req.url, true).query;
	query = JSON.stringify(query);
	console.log("query : " + query);
	var str = JSON.parse(query);
	console.log("str.path : " + str.path);
	var path = str.path;
			
	fs.readFile(__dirname + '/html/mkdir.html','utf8', function(error, data){
		res.send(ejs.render(data, {devID : devID, path : path}));
	});
});

app.post('/login', userInfo.login);
app.post('/authCode', userInfo.authCode);
app.get('/list/:deviceID', fileInfo.listOpen);
app.get('/webRead/:fileID', fileInfo.webRead);
app.post('/write2', fileInfo.write2);
app.post('/mkdir2', fileInfo.mkdir2);
app.post('/remove2', fileInfo.remove2);
app.post('/rmdir2', fileInfo.rmdir2);

//image page route
app.get('/img/:fileName', function(req, res){
	var fileName = req.params.fileName;
	console.log("fileName : " + fileName);

	fs.readFile(__dirname + '/html/img/' + fileName, function(err, data){
		res.writeHead(200, {'Content-Type' : 'text/html'});
		res.end(data);
	});
});

//js page route
app.get('/js/:jsName', function(req, res){
	var jsName = req.params.jsName;
	
	fs.readFile(__dirname + '/html/js/' + jsName, function(err, data){
		res.writeHead(200, {'Content-Type' : 'text/javascript'});
		res.end(data);
	});
});

//css page route
app.get('/css/:cssName', function(req, res){
	var cssName = req.params.cssName;
	
	fs.readFile(__dirname + '/html/css/' + cssName, function(err, data){
		res.writeHead(200, {'Content-Type' : 'text/css'});
		res.end(data);
	});
});

//android
app.get('/userList', userInfo.listOfUser);
app.get('/addUser/:id/:gcm', userInfo.addUser);
app.delete('/removeUser/:id', userInfo.removeUser);

app.get('/access/:fileid', fileInfo.access);
app.get('/read/:fileid', fileInfo.read);
app.post('/create', fileInfo.create)
app.post('/write/:fileid/:fileSize/:offSet', fileInfo.write);
app.delete('/remove/:fileid', fileInfo.remove);
app.get('/opendir/:deviceid', fileInfo.opendir);

app.post('/mkdir', fileInfo.mkdir);
app.delete('/rmdir/:folderid', fileInfo.rmdir);

// development only
if ('development' == app.get('env')) {
  app.use(express.errorHandler());
}

http.createServer(app).listen(app.get('port'), function(){
  console.log('Express server listening on port ' + app.get('port'));
});
