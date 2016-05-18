var async = require('async');
var http = require('http');
var fs = require('fs');

var req_web_service = 
{
	address : 'localhost',
	port : 4000
}


// call face verification service
checkPair = function(i1Path, i2Path, _cb)
{	
	console.log('preparing.....');
	console.log(i1Path);
	console.log(i2Path);

	var img1 = fs.readFileSync(i1Path);
	console.log('read first image ' + img1.length);

	var img2 = fs.readFileSync(i2Path);
	console.log('read second image ' + img2.length);

	var req_data = JSON.stringify
				(
					{
						'requestVersion' : 'v2' ,
						'priority' : 'high' ,
						'input' : 
							[
								{
								type : 'json',
								data : img1,
								},
								{
								type : 'json',
								data : img2,
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
								console.log( 'STATUS: ' + response.statusCode);
								console.log( 'HEADERS: ' + JSON.stringify(response.headers));
								
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
												_cb(null, response_data);
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
									_cb('timeout', null);
								}
							);
						}
				);
			
			http_request.on
				(	
					'error', 
					function ( error ) // Error handling here
					{
						_cb(error, null);
					}
				);
			
			//console.log('sending data ' + req_data);

			http_request.write( req_data );	
			http_request.end();
}

processPair = function(i1Path, i2Path, _cb)
{
	checkPair(i1Path, i2Path, 
		function(err, results) 
		{
			var output = 
			{
				serviceOutput : "",
				label : -1 
			};

			if (err != null) 
			{
				output.serviceOutput = err;
			} else {
				output.serviceOutput = results;
				output.label = JSON.parse(results).areIdentical;
			}

			console.log('result is ' + JSON.stringify(output));

			_cb(output);
		});
}


var faceVerification = 
{
	name : 'faceVerification',
	processPair : processPair,
}

module.exports = faceVerification