var mongo = require('mongodb');
var fs = require('fs');
var url = require('url');
var ejs = require('ejs');

var Server = mongo.Server,
    Db = mongo.Db,
    BSON = mongo.BSONPure;

var server = new Server('localhost', 27017, {auto_reconnect: true});
db = new Db('darkcloud', server);

db.open(function(err, db) {
		if(!err){
			console.log("Connected to FileInfo Database");
			db.collection('fileInfo', {safe : true}, function(err, collection) {
				if(err){
					console.log("FileInfo Collection doesn't exist. create New Collection.");
				}
			});
		}
});

exports.webRead = function(req, res){
	var fileID = req.params.fileID;
	var obj, parse;
	var readStream;
	
	//console.log(req);
	db.collection('fileInfo', function(err, collection) {
			find = collection.find({'_id': new BSON.ObjectID(fileID)}, {'fileName' : 1, '_id' : 1});
			find.count(function(err, item){
				if(item == 0){ 
					console.log("file does not exist!");
					res.send("0");
				}

				else {
					find.toArray(function(err, data){
						console.log(data[0]);
						obj = JSON.stringify(data[0]);
						parse = JSON.parse(obj);
						readStream = fs.createReadStream("/home/hb/DarkCloud/src/server/fileStorage/" + parse._id);
						if( parse.fileName.indexOf('.png') > 0 || parse.fileName.indexOf('.jpg') > 0
						|| parse.fileName.indexOf('.jpeg') > 0 || parse.fileName.indexOf('.gif') > 0 
						|| parse.fileName.indexOf('.bmp') > 0 || parse.fileName.indexOf('.PNG') > 0 
						|| parse.fileName.indexOf('.JPG') > 0 || parse.fileName.indexOf('.JPEG') > 0 
						|| parse.fileName.indexOf('.GIF') > 0 || parse.fileName.indexOf('.BMP') > 0){
							readStream.pipe(res);

						}else{

							res.writeHead(200, {'Content-Type' : 'Application/octet-stream',
								'Content-Disposition' : 'inline; filename=' + parse.fileName + ';'});
							readStream.pipe(res);
						}
					}); 
				}
			});
	});

};

exports.listOpen = function(req, res){ // 웹 페이지와 DB 연결
	var devID = req.params.deviceID;

	var query = url.parse(req.url, true).query;
	query = JSON.stringify(query);
	console.log("query : " + query);
	var str = JSON.parse(query);
	console.log("path : " + str.path);

	fs.readFile('/home/hb/DarkCloud/src/server/html/list.html','utf8', function(error, data) {
		db.collection('fileInfo', function(err, collection) {
			find = collection.find({deviceID : devID, path : str.path}).sort({isFolder:-1});
					
				find.toArray(function(err, results){
					
					res.send(ejs.render(data, {data : results, path : str.path , devID : devID}));
				});
		});
	});
};

exports.access = function(req, res) { // 파일이 존재하는지 확인해주는 함수
   var fileid = req.params.fileid;
   var find;
   
   db.collection('fileInfo', function(err, collection) {
		find = collection.find({'_id' : new BSON.Objectid(fileid)}, {'fileName' : 1, '_id' : 1});
		find.count(function(err, item) {
			if(item == 0){
				console.log("file does not exist!");
				res.send("0");
			} else {
				console.log("file exist!");
				res.send("1");

			}
		});
	});

};


exports.read = function(req, res) { // 디바이스가 파일을 요청할 때
	var fileid = req.params.fileid;
	console.log(fileid);
	var obj, parse;
	var readStream;

	//console.log(req);
	db.collection('fileInfo', function(err, collection) {
			find = collection.find({'_id': new BSON.ObjectID(fileid)}, {'fileName' : 1, '_id' : 1});
			find.count(function(err, item){
				if(item == 0){ 
					console.log("file does not exist!");
					res.send("0");
				}

				else {
					find.toArray(function(err, data){
						console.log(data[0]);
						obj = JSON.stringify(data[0]);
						parse = JSON.parse(obj);
						readStream = fs.createReadStream("/home/hb/DarkCloud/src/server/fileStorage/" + parse._id);
						readStream.pipe(res);
					}); 
				}
			});
	});
};

exports.rename = function(req, res){
	var fileid = req.params.fileid;
	var rename = req.params.rename;

	db.collection('fileInfo', function(err, collection) {
		collection.update({_id : new BSON.ObjectID(fileid)}, {$set: {fileName : rename}});
	});
};

exports.write = function(req, res) { // 디바이스가 파일을 저장할 때(안드로이드)
	var fileid = req.params.fileid;
	var fileSize = req.params.fileSize;
	var find;
	var body = JSON.stringify(req.body);
	body = body.substring(2, body.length - 5).toString();
	var decodeBody = new Buffer(body.length/2);
	var obj, parse;

	// Hex 변환
	for(i = 0 ; i < body.length ; i += 4 ){
		var tmp = body.charCodeAt(i + 3);
		var tmp2 = body.charCodeAt(i + 2);
		var tmp3 = body.charCodeAt(i + 1);
		var tmp4 = body.charCodeAt(i);

		tmp = tmp - 48;
		tmp2 = tmp2 - 48;
		tmp3 = tmp3 - 48;
		tmp4 = tmp4 - 48;

		if(tmp > 16)	tmp = tmp - 7;
		if(tmp2 > 16)	tmp2 = tmp2 - 7;
		if(tmp3 > 16)	tmp3 = tmp3 - 7;
		if(tmp4 > 16)	tmp4 = tmp4 - 7;

		tmp = tmp * 16;
		tmp2 = tmp + tmp2;
		
		tmp3 = tmp3 * 16;
		tmp4 = tmp4 + tmp3;

		decodeBody[i/2] = tmp4;
		decodeBody[i/2+1] = tmp2;
	}

	fs.appendFile("/home/hb/DarkCloud/src/server/fileStorage/" + fileid, decodeBody, 'utf8', function(err){
				if(err) throw err;
	});
	
	db.collection('fileInfo', function(err, collection) {
		find = collection.find({_id : new BSON.ObjectID(fileid)}, {fileSize : 1});
		find.toArray(function(err, result) {
			//console.log(data);
			obj = JSON.stringify(result[0]);
			parse = JSON.parse(obj);
			console.log(result[0]);
			fileSize = parseInt(fileSize) + parseInt(parse.fileSize);

			collection.update({_id : new BSON.ObjectID(fileid)}, {$set: { fileSize : fileSize } } );
		});
		res.send("write_success");
	});
};

exports.create = function(req, res){
	var body = req.body;
	var obj = JSON.stringify(body);
	var parse = JSON.parse(obj);
	var parse2;
	var find, find2;
	var tmp, tmp2;
	
	console.log("fileName : " + parse.fileName + ", fileSize : " + parse.fileSize +  ", uploadDate : "  + parse.uploadDate + ", deviceID : " + parse.deviceID + ", isFolder : " + parse.isFolder +  ", path : " + parse.path);

	db.collection('fileInfo', function(err, collection) {
		find = collection.find({"fileName" : parse.fileName, "path" : parse.path});
		
		find.count(function(err, count){
			if(count >= 1){
				find.toArray(function(err, result){
			 		tmp = JSON.stringify(result[0]);
					console.log(tmp);
			 		tmp2 = JSON.parse(tmp);
					console.log("File exist");
					res.send(tmp2._id);
				});
			}
			else {
				collection.insert({"fileName" : parse.fileName, "fileSize" : parse.fileSize , "uploadDate" : parse.uploadDate , "deviceID" : parse.deviceID , "isFolder" : parse.isFolder, "path" : parse.path}, {safe:true}, function(err, result) {
					find2 = collection.find({"fileName" : parse.fileName, "path" : parse.path});
					find2.toArray(function(err, result) {
						//console.log(data);
						obj2 = JSON.stringify(result[0]);
						parse2 = JSON.parse(obj2);
						fs.writeFile("/home/hb/DarkCloud/src/server/fileStorage/" + parse2._id , "", function(err){
							if(err) console.log(err);
							else	console.log('File create success');
						});
						console.log("return : " + parse2._id);
						res.send(parse2._id);
					});
				});	
			}
		});
	});
};

exports.write2 = function(req, res) { // 디바이스가 파일을 저장할 때(웹)
	/* 헤더를 DB에 저장 */
	var find;
	var date = new Date();
	var getTime = date.getTime()/1000;
	var body = req.body;
	var obj = JSON.stringify(body);
	var parse = JSON.parse(obj);
	var obj2, parse2;
	console.log("parse.path : " + parse.path + " , parse.devID : " + parse.devID);

	fs.readFile(req.files.uploadFile.path, function(error, data) {
			db.collection('fileInfo', function(err, collection) {
				collection.insert({"fileName" : req.files.uploadFile.name, "fileSize" : req.files.uploadFile.size, "uploadDate" : getTime , "deviceID" : parse.devID , "isFolder" : 0, "path" : parse.path}, {safe:true}, function(err, result) {
				
					find = collection.find({"fileName" : req.files.uploadFile.name, "path" : parse.path});
					find.toArray(function(err, result) {
						//console.log(data);
						obj2 = JSON.stringify(result[0]);
						parse2 = JSON.parse(obj2);
						fs.writeFile("/home/hb/DarkCloud/src/server/fileStorage/" + parse2._id ,data, function(err){
							if(err) console.log(err);
							else	console.log('File Write success');
						});
						console.log("return : " + parse2._id);
						res.send(parse2._id);
					});
				});
			});
	});
};


exports.remove = function(req, res) {
	var fileid = req.params.fileid;
	var path;
	console.log('Deleting File : ' + fileid);
	db.collection('fileInfo', function(err, collection) {
			collection.remove({'_id' : new BSON.ObjectID(fileid)}, {safe:true}, function(err, result){
				if(err){
					res.send('delete_error');
				} else{
					fs.unlink("/home/hb/DarkCloud/src/server/fileStorage/" + fileid, function(err){
						if(err)	console.log(err);
						else console.log("file remove sucess!");
					});
					console.log('delete success');
					res.send("delete_success");
				}
			});
			
	});
};

exports.opendir = function(req, res) { // 디바이스가 파일, 폴더 리스트를 요청할 때
	var deviceid = req.params.deviceid;
	var query = url.parse(req.url, true).query;
	query = JSON.stringify(query);
	var str = JSON.parse(query);
	console.log(str.path);

	db.collection('fileInfo', function(err, collection) {
		collection.find({'deviceID' : deviceid, 'path' : str.path}, {'data' : 0}).toArray(function(err, items) {
			if(err) res.send('list_err');
			console.log(items);
			res.send(items);
		});

	});

};


exports.mkdir = function(req, res) { // Android
	var body = req.body;
	var obj, obj2, parse, parse2;
	obj = JSON.stringify(body);
	parse = JSON.parse(obj);
	

	console.log("fileName : " + parse.fileName + ", fileSize : " + parse.fileSize +  ", uploadDate : "  + parse.uploadDate + ", deviceID : " + parse.deviceID + ", isFolder : " + parse.isFolder +  ", path : " + parse.path);
	
	db.collection('fileInfo', function(err, collection) {
		collection.insert(body, {safe:true}, function(err,result) {
			if(err) {
				res.send('mkdir_dberror');
			} else{
				console.log("mkdir_success");
				find = collection.find({"fileName" : parse.fileName, "path" : parse.path});
				console.log(parse.fileName + ", " + parse.path);
				find.toArray(function(err, data) {
						console.log(data);
						obj2 = JSON.stringify(data[0]);
						parse2 = JSON.parse(obj2);
						console.log("return : " + parse2._id);
						res.send(parse2._id);
				});

			}	
		});
	});
};

exports.mkdir2 = function(req, res) { // Web
	var body = req.body;
	var obj, parse, obj2, parse2;
	//console.log(body);
	
	var date = new Date();
	var getTime = date.getTime()/1000;
	
	obj = JSON.stringify(body);
	parse = JSON.parse(obj);

	console.log("parse.path : " + parse.path + ", parse.devID : " + parse.devID + ", dirName : " + parse.dirName);

	db.collection('fileInfo', function(err, collection) {
		//collection.insert(body, {safe:true}, function(err,result) {
		collection.insert({"fileName" : parse.dirName, "fileSize" : "0", "uploadDate" : getTime , "deviceID" : parse.devID , "isFolder" : "1", "path" : parse.path}, {safe:true}, function(err, result) {
			if(err) {
				res.send('mkdir_dberror');
			} else{
				console.log("mkdir_success");
				find = collection.find({"fileName" : parse.dirName, "path" : parse.path});
				find.toArray(function(err, data) {
						console.log(data);
						obj2 = JSON.stringify(data[0]);
						parse2 = JSON.parse(obj2);
						console.log("return : " + parse2._id);
						res.send(parse2._id);
				});
			}	
		});
	});
};
exports.remove2 = function(req, res){
	var body, obj, parse, find;
	body = req.body;
	obj = JSON.stringify(body);
	parse = JSON.parse(obj);

	db.collection('fileInfo', function(err, collection){
		fs.unlink("/home/hb/DarkCloud/src/server/fileStorage/" + parse._id, function(err){
			if(err)	console.log(err);
			else console.log("file remove sucess!");
		});
		collection.remove({'_id' : new BSON.ObjectID(parse._id)}, {safe:true}, function(err, result){
			if(err) res.send("remove_file_error");
			else res.send("remove_file_success");
		});
	});

};
exports.rmdir2 = function(req, res){
	var body, obj, parse, find;
	body = req.body;
	obj = JSON.stringify(body);
	parse = JSON.parse(obj);

	db.collection('fileInfo', function(err, collection) {

	find2 = collection.find({deviceID : devID});

	/*	find2.each(function(err, data){
			obj2 = JSON.stringify(data);
			parse2 = JSON.parse(obj2);
			if( parse2.path.indexOf((parse.path + parse.fileName)) > 0 ){ // 이게 들어간 path를 다 찾으면 다 지운다.
				collection.remove({'_id' : new BSON.ObjectID(parse2._id)}, {safe:true}, function(err, result){
					if(err)	res.send('rmdir_hawi_dberror');
					else	console.log('remove_all_success');
				});
			}
		});*/
		collection.remove({'_id' : new BSON.ObjectID(parse._id)}, {safe:true}, function(err, result){
			if(err)	res.send("remove_folder_error");
			else res.send("remove_folder_sucess");
			});
	});
};

exports.rmdir = function(req, res) {
	var folderid = req.params.folderid;

	db.collection('fileInfo', function(err, collection) {

		collection.remove({'_id' : new BSON.ObjectID(folderid)}, {safe:true}, function(err, result){
			if(err){
				res.send('rmdir_dberror');
			} else{
				console.log("rmdir_success");
				res.send(result);
			}
		});
	});
};
