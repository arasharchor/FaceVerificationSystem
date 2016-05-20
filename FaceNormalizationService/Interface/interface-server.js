var fs = require('fs');
var path = require('path');
var http = require('http');
var express = require('express');
var util = require('util');

var head = '<!DOCTYPE html>                                                                              \
<html lang="en"> 																						 \
<head> 																									 \
  <title>Face Normalization Interface</title> 															 \
  <meta charset="utf-8"> 																				 \
  <meta name="viewport" content="width=device-width, initial-scale=1"> 									 \
  <link rel="stylesheet" href="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css"> 	 \
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.2/jquery.min.js"></script> 	         \
  <script src="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js"></script> 			 \
  <style> 																								 \
  .carousel-inner > .item > img, 																		 \
  .carousel-inner > .item > a > img { 																	 \
      margin: auto; 																					 \
  } 																									 \
  .base-container {																						 \
  	max-width: 100%;                                                                                     \
  	max-height: 100%;                                                                                    \
  } 																									 \
  .center-text{ 																						 \
  	text-align: center; 																				 \
  }																										 \																						 \
  .cell { 																								 \
   	display: table-cell; 																			     \
   	float: none; 																						 \
  } \
  </style> 																								 \
</head>																							         \
<body style="background-color: rgb(225,230,235);"> 														 \
  <h1>Face Normalization Interface</h1> 																 \
  <form method="post" enctype="multipart/form-data" action="/upload"> 									 \
     <input type="file" name="left_image"> 																 \
     <input type="submit" value="Submit"> 																 \
  </form>' 																								 

var end = '\
</body>    \
</html>'

var req_web_service = 
	{ 
	address:	'localhost',
	port: 5000,
	path: '/normalize2D/'
	};

var output_prefix     = '[ Interface FN]';
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
	var html = head + ' <pre>' + JSON.stringify(json) + '</pre>\n' + end
	return html;
}


// send classification request to Face Normalization Interface service
var normalizationRequest = function(inputPath, inputName, _cb)
{
	var imgContent = fs.readFileSync( inputPath ).toString('base64');
	
	var req_data = JSON.stringify
				(
					{
						'requestVersion' : 'v2' ,
						'priority' : 'high' ,
						'input' : 
							[
								{
								type : 'json',
								data : imgContent,
								filename : inputName
								}
							],

						'action' : 
							{											
							method : "2D",
							options : {},
							}
					}
				);

	var http_request_options = 
				{
				  host: req_web_service.address
				, port: req_web_service.port
				, method: 'POST'
				, path: req_web_service.path
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

function normalizeFace( req, res )
{
	var imgPath  = req.files.image.path;
	var imgName  = req.files.image.name;
	
	normalizationRequest(imgPath, imgName, function (success, response) {
		if (success == true)
		{
			try {
				response = JSON.parse(response)
			}
			catch(ex) {
				res.send(response)
				return
			}

			var html = head + 
		   '<div class="row"> \
			<div class="col-md-6"> \
			<h3 class="center-text">Original image</h4> \
			</div> \
			<div class="col-md-6"> \
			<h3 class="center-text">Extracted faces</h3> \
			</div>  \
			</div> \
			<div class="row">                                                 \
  				<div class="col-md-6">'

  			html += '<img class="base-container" src=\'data:image/png;base64,' + response.originalImage + '\'/>'
			html +='</div>'

			if (response.faces.length > 0) {
	  			html +=' \
	  				<div class="col-md-6"> 											  \
	  					<div class="container base-container">											  \
	  						<br>															  \
	  						<div id="myCarousel" class="carousel slide" data-ride="carousel"> \
	    					<!-- Indicators -->												  \
	    					<ol class="carousel-indicators">'
	    		
	    		for (var i = 0; i < response.faces.length; i++)
	      			html += util.format('<li data-target="#myCarousel" data-slide-to="%d"></li>', i);
	            
	            html += ' 				\
	            </ol> 					\
	            						\
	    		<!-- Wrapper for slides --> \
	    		<div class="carousel-inner" role="listbox">'

	    		for (var i = 0; i < response.faces.length; i++) {
	      			if (i == 0)
	      				html += '<div class="item active">'
	      			else
	      				html += '<div class="item">'
	        		html += '	<img src=\'data:image/png;base64,' + response.faces[i] + '\'/>'
	      			html += '</div>'
	      		}

	     		html += ' 																					\
				     <!-- Left and right controls --> 														\
				   <a class="left carousel-control" href="#myCarousel" role="button" data-slide="prev"> 	\
				      <span class="glyphicon glyphicon-chevron-left" aria-hidden="true"></span> 			\
				      <span class="sr-only">Previous</span> 												\
				    </a> 																					\
				    <a class="right carousel-control" href="#myCarousel" role="button" data-slide="next"> 	\
				      <span class="glyphicon glyphicon-chevron-right" aria-hidden="true"></span> 			\
				      <span class="sr-only">Next</span> 													\
				    </a> 																					\
				  </div> 																					\
				</div>																						\
				</div>															  							\
	  			</div>'
	  		} else {
	  			html +='<div class="col-md-6 cell"> \
	  			 		 	<img class="center-img" src="empty_set.png"/> \
	  			 		</div>'							
	  		}
			
			html += end	

			res.send(html);
			
		} else {
			server_log('error : ' + JSON.stringify(response));
			res.send(get_err_res(response));
		}
	});
}

function postdata( req, res )
{
	var files = { image : null};
	var fstream;
    
	req.pipe(req.busboy);
    req.busboy.on(	
    	'file', 
		function (fieldname, file, filename) 
		{
			server_log("Uploading: filename: " + filename);
			fstream = fs.createWriteStream(__dirname + '/temp/' + filename);
			file.pipe(fstream);
			fstream.on(	'close', 
						function () 
							{
							if( files.image == null )
							{
								files.image = { path: './temp/' + filename, name : filename};
							}

							req.files = files;
							
							server_log('Normalizing faces ' + JSON.stringify(req.files));
							normalizeFace(req, res);
							
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
		res.sendFile('index.html');
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
		console.log( 'Face Normalization Interface interface server listening at http://%s:%s', host, port );
	});