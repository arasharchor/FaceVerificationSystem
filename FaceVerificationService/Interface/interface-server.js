var fs = require('fs');
var path = require('path');
var http = require('http');
var express = require('express');

// main index html 
var html = '<!DOCTYPE html>\n';
	html += '<html>\n';
	html += '<body style="background-color: rgb(225,230,235);" >\n';
	html += '  <h1>Face verification</h1>\n';
	html += '  <form method=\"post\" enctype=\"multipart/form-data\" action=\"/upload\">\n';
    html += '     <input type=\"file\" name=\"left_image\">\n'
    html += '     <input type=\"file\" name=\"right_image\"></br></br>\n'
    html += '     <input type=\"submit\" value=\"Submit\">\n';
	html += '  </form>\n';
	html += '</body>\n';
	html += '</html>';

var req_web_service = 
	{ 
	address:	'localhost',
	port: 5000 
	};

var output_prefix     = '[ Interface FV]';
var EXPRESS_VERSION   = null;
var global_file_index = 0;

// log message
function server_log( msg )
{
	var date = new Date();
	console.log( '[' + date.toUTCString() + '] ' + output_prefix + msg );
}


function check_json( json )
{
	if(    ( json )
		&& ( 'status' in json )
		&& ( json.status )
		&& ( "ok" == json.status )
		&& ( 'result' in json )
		&& ( json.result )
		&& ( 'face' in json.result )
		&& ( json.result.face )
		&& ( 'image' in json.result )
		)
		{
		server_log('Response json is OK');
		return 0;
		}
	else{
		server_log('Response json is not OK' + JSON.stringify(json));
		return -1;
		}	
}

// return error
function get_err_res( json )
{	
	var html = '<!DOCTYPE html>\n';
	html += '<html>\n';
	html += '<body style="background-color: rgb(225,230,235);" >\n';
	html += '  <h1>Face verification</h1>\n';	
	html += '  <form method=\"post\" enctype=\"multipart/form-data\" action=\"/upload\">\n';
    html += '     <input type=\"file\" name=\"left_image\">\n';
    html += '     <input type=\"file\" name=\"right_image\"></br></br>\n'
    html += '     <input type=\"submit\" value=\"Submit\">\n';
	html += '  </form>\n';
	html += ' <p> Web request error with response : </p>\n';
	html += ' <pre>' + JSON.stringify(json) + '</pre>\n';
	html += '</body>\n';
	html += '</html>';
	return html;
}


// send classification request to face verification service
var calssifiyRequest = function(input1, input2, _cb)
{
	var leftImgContent = fs.readFileSync( input1 ).toString('base64');
	var rightImgContent = fs.readFileSync( input2 ).toString('base64');

	var req_data = JSON.stringify
				(
					{
						'requestVersion' : 'v2' ,
						'priority' : 'high' ,
						'input' : 
							[
								{
								type : 'json',
								data : leftImgContent,
								},
								{
								type : 'json',
								data : rightImgContent,
								},
							],

						'action' : 
							{											
							method : "face_compare",
							options : {},
							}
					}
				);


	var http_request_options = 
				{
				  host: req_web_service.address
				, port: req_web_service.port
				, method: 'POST'
				, path: '/'
				, headers:	{ 
							  'Content-Type' : 'application/json'
							, 'Content-Length' : Buffer.byteLength( req_data, 'utf8' )
							}
				};

	var http_request = http.request
						(
							http_request_options , 
							function( response ) 
								{
								console.log( output_prefix, 'STATUS: ' + response.statusCode);
								console.log( output_prefix, 'HEADERS: ' + JSON.stringify(response.headers));
								
								var response_data = '';
								  
								response.on(	
											'data', 
											function( chunk ) 
												{
												response_data += chunk;
												}
										);

								response.on(	
											'end', 
											function () 
											{
												_cb(true, response_data);
											}
										);								
								}
						);
						
			//---------------------------------------//
			
			http_request.on
				(
					'socket', 
					function (socket) 
						{
						socket.setTimeout( 60 * 60 * 1000 );  
						socket.on
							(
								'timeout', 
								function() 
									{
									_cb(false, 'Request timeout');
									http_request.abort();
									}
							);
						}
				);
			
			http_request.on
				(	
					'error', 
					function ( error ) // Error handling here
						{
						console.log( output_prefix, "http_request.on('error'):", error );
						html = html.replace( /_INNER_HTML_/g, "ERROR: "+error );
						
						var date_end = new Date();
						console.log( output_prefix, "[app.post('/')] END ("+date_end.toUTCString()+")" );										
						_cb( false, html );
						}
				);
				
			//---------------------------------------//
						
			http_request.write( req_data ); //This is the data we are posting, it needs to be a string or a buffer					
			http_request.end();
}

function compareFaces( req, res )
{
	var left_img  = req.files.left_image.path;
	var right_img = req.files.right_image.path;

	calssifiyRequest(left_img, right_img, function (success, response) {
		if (success == true)
		{
			server_log('classification response : ' + JSON.stringify(response));
			
			var html = '<!DOCTYPE html>\n';
			html += '<html>\n';
			html += '<body style="background-color: rgb(225,230,235);" >\n';
			html += '  <h1>Face verification</h1>\n';	
			html += '  <form method=\"post\" enctype=\"multipart/form-data\" action=\"/upload\">\n';
		    html += '     <input type=\"file\" name=\"left_image\">\n';
		    html += '     <input type=\"file\" name=\"right_image\"></br></br>\n'
		    html += '     <input type=\"submit\" value=\"Submit\">\n';
			html += '  </form>\n';
			html += ' <p> Face verification response: </p>\n';
			html += ' <pre>' + response + '</pre>\n';
			html += '</body>\n';
			html += '</html>';

			res.send(html);
			
		} else {
			server_log('error : ' + JSON.stringify(response));

			res.send(get_err_res(response));
		
		}
	});
}

function postdata( req, res )
{
	var files = { left_image : null, right_image: null };
	var fstream;
    
	req.pipe(req.busboy);
    req.busboy.on(	
    	'file', 
		function (fieldname, file, filename) 
		{
			server_log("Uploading: filename: " + filename + ' fieldname: ' + fieldname);
			fstream = fs.createWriteStream(__dirname + '/files/' + filename);
			file.pipe(fstream);
			fstream.on(	'close', 
						function () 
							{
							if( files.left_image == null )
								{
								files.left_image = { path: 'files/' + filename };
								}
							else{
								if( files.right_image == null )
								{
									files.right_image = { path: 'files/' + filename };
									req.files = files;
									server_log('comparing faces ' + JSON.stringify(req.files));
									compareFaces(req, res);
								}
							}
						});
		});
    
    req.busboy.on(	
    	'error', 
		function ( error )
		{
			server_log('Busboy error: ' + error);
			res.send( get_err_res('Internal error: busboy') );
		});
}


// express server info
var app = express();
var busboy = require('connect-busboy');

app.use(busboy()); 
app.use( "/tmp", express.static( "/tmp") );
app.use( express.static( __dirname + '/' ) );
			
var errorHandler = require('errorhandler');
app.use( errorHandler() );

app.get(    
	'/', 
	function( req, res ) 
	{
		res.send( html );
	});

app.post(
	'/upload', 
	function( req, res ) 
	{
		postdata(req, res);
	});



var server = app.listen(	
	4400, 
	function() 
	{
		var host = server.address().address;
		var port = server.address().port;
		console.log( 'Face verification interface server listening at http://%s:%s', host, port );
	});