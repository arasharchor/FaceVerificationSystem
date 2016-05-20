var fs = require('fs');
var path = require('path');
var http = require('http');
var express = require('express');
var bodyParser = require('body-parser')

// express server info
var app = express();
//app.use(bodyParser.json());
app.use(bodyParser.json({limit: '100mb'}));
app.use(bodyParser.urlencoded({limit: "100mb", extended: true, parameterLimit:10000}));

app.use( "/tmp", express.static( "/tmp") );
app.use( express.static( __dirname + '/' ) );
			
var errorHandler = require('errorhandler');
app.use( errorHandler());


output_prefix = '[ Service FV ] '
// log message
function server_log( msg )
{
	var date = new Date();
	console.log( '[' + date.toUTCString() + ']' + output_prefix + msg );
}


var ffi = require('ffi')
var ref = require('ref')
var int = ref.types.int
var str = ref.types.CString


var lib = ffi.Library('./build/Debug/FaceVerification', {
  'compareImages': [ int, [str, str] ],
  'double' : [int, [str, str]]
});

app.post(    
	'/', 
	function( req, res ) 
	{
		console.log('received a face verification request...');

		var data = req.body.input;
		var img1 = data[0].data;
		var img2 = data[1].data;

		var decoded1 = new Buffer(img1, 'base64');
		var decoded2 = new Buffer(img2, 'base64');

		var id = new Date().getTime();
		var filename1 = '.\\tmp\\image_1_' + id + '.jpg';
		var filename2 = '.\\tmp\\image_2_' + id + '.jpg';

		fs.writeFileSync(filename1, decoded1);
		fs.writeFileSync(filename2, decoded2);

		console.log('passing files to FV library');

		// get result from service
		var diff = lib.compareImages(filename1, filename2);

		fs.unlink(filename1, function(err) {});
		fs.unlink(filename2, function(err) {});

		var result = {
			areIdentical : diff
		}

		res.send(JSON.stringify(result));
	});

// initialize library
var landmarks_path = 'models_data\\landmarks_points.dat'
var models_path = 'models_data\\lfw_train_10\\normalized'

server_log('Loading FV lib models: ' + models_path);
var ret = lib.init(landmarks_path, models_path);

if (ret === -1)
	process.exit(1);

server_log('Loading finished.');

var server = app.listen(	
	4000, 
	function() 
	{
		var host = server.address().address;
		var port = server.address().port;
		server_log( 'FV service listening at http://%s:%s', host, port );
	});