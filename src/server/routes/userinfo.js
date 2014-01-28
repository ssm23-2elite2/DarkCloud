var mongo = require('mongodb');
var GCM = require('gcm').GCM;

var Server = mongo.Server,
	Db = mongo.Db,
	BSON = mongo.BSONPure;


var server = new Server('localhost', 27017, {auto_reconnect: true});
db = new Db('darkcloud', server);

db.open(function(err, db) {
		if(!err) {
			console.log("Connected to Userinfo Database");
			db.collection('userInfo', {safe : true}, function(err, collection) {
				if(err){
					console.log("The Userinfo Collection doesn't exist. create new Collection.");
				}
			});
		}
});

exports.login = function(req, res) { // login 화면
	var body = req.body;
	var obj = JSON.stringify(body);
	var parse = JSON.parse(obj);
	var id = parse.id;
	var find, obj2, parse2;
	var apiKey = 'AIzaSyAt6i26Yl4qLQq1n272qDvRKAF1hq2O960';
	var gcmKey, message;
	var gcm = new GCM(apiKey);

	// 인증번호 생성(4자리 난수)
	var random = (parseInt( (Math.random() * 9) + 1 ) + parseInt( ((Math.random() * 9) + 1) * 10 ) + parseInt( ((Math.random() * 9) + 1) * 100) + parseInt(((Math.random() * 9) + 1) * 1000));
	console.log(random);

	db.collection('userInfo', function(err, collection) {
		find = collection.find({deviceID : id});

		find.count(function(err, count){
			if(count == 0){
				console.log("id not exist!!");
				res.send("id_not_exist");
			}else {
				find.toArray(function(err, user) {
					obj2 = JSON.stringify(user[0]);
					parse2 = JSON.parse(obj2);
					gcmKey = parse2.gcmKey;
					message = {
						registration_id : gcmKey,
						'data.msg' : ('인증번호 : ' + random)
					};
					console.log(message);
					gcm.send(message, function(err, messageId){
						if(err)	console.log("Something has gone wrong!!");
						else	console.log("Send With Message ID : ", messageId);
					});		
				});

				// 난수 저장
				collection.update({deviceID : id}, {$set : {authCode : random}});
				res.send("enter_authCode");
			}
		});
	});

};

exports.authCode = function(req, res) { // 인증번호 검사
	var body = req.body;
	console.log(body);
	var obj = JSON.stringify(body);
	var parse = JSON.parse(obj);
	var authCode = parseInt(parse.authCode);
	var find, deviceID;
	
	console.log("authCode : " + authCode);

	db.collection('userInfo', function(err, collection){
		var obj2, parse2;

		find = collection.find({authCode : authCode});
		
		
		find.count(function(err, data){
			if(data == 0){
				console.log("Authentication Code is Not Equal!!");
				res.send("authCode_not_equal");
			} else{
				find.toArray(function(err, data){
					obj2 = JSON.stringify(data[0]);
					parse2 = JSON.parse(obj2);
					deviceID = parse2.deviceID;
					console.log(data);
					console.log("Authentication Complete.");
					console.log(deviceID);

					collection.update({deviceID : deviceID}, {$set : {authCode : ""}});

					res.send(deviceID);
				});

				
			}

		});
	});
};


exports.listOfUser = function(req, res) { // user List
	db.collection('userInfo', function(err, collection) {
			collection.find().toArray(function(err, items) {
				res.send(items);
			});
	});
};

exports.addUser = function(req, res) { // user 추가
	var id = req.params.id;
	var apiKey = 'AIzaSyAt6i26Yl4qLQq1n272qDvRKAF1hq2O960';
	var gcmKey = req.params.gcm;
	var gcm = new GCM(apiKey);

	var find, tmp;
	
	var message = {
		registration_id : gcmKey,
		'data.msg' : 'registration complete'
	};

	console.log('Adding New User = ' + id);

	db.collection('userInfo', function(err, collection) {
			find = collection.find({deviceID : id});
			
			find.count(function(err, item) {
				if(item > 0)
				{
					console.log(item + " - id exist, new gcm Key update"); // id가 존재하면 gcm key update

					collection.update({deviceID : id}, {$set : {gcmKey : gcmKey}});

					gcm.send(message, function(err, messageId){
						if(err)	console.log("Something has gone wrong!!");
						else	console.log("Send With Message ID : ", messageId);
					});
					find.toArray(function(err, item) { 
						res.send(item);
					});
				} else
				{
					// 아이디 디비 저장
					collection.insert({"deviceID" : id, "gcmKey" : gcmKey, "authCode" : ""}, {safe:true}, function(err, result) {
						if(err) {
							res.send({'error' : 'An error has occurred'});
						} else {
							console.log('Success :' + JSON.stringify(result[0]));
							gcm.send(message, function(err, messageId){
								if(err)	console.log("Something has gone wrong!!");
								else	console.log("Send With Message ID : ", messageId);
							});
							res.send(result[0]);
						}
					});
				}
			});
		});
};

exports.removeUser = function(req, res) {
	var id = req.params.id;
	console.log('Deleting user : ' + id);
	db.collection('userInfo', function(err, collection) {
			collection.remove({"deviceID" : id}, {safe:true}, function(err, result) {
				if(err){
					res.send({'error' : 'An Error has occurred - ' + err});
				} else {
					console.log('' + result + ' document(s) deleted' );
					res.send(req.body);
				}
			});
		});
};
