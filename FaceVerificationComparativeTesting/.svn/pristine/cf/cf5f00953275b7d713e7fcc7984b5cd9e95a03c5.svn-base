var async = require('async');
var http = require('http');
var fs = require('fs');

var RECOGNITION_TRESHOLD = 0.5;

var faceDetection = function(image, callback)
{
	var options = 
	{
		internal :
		{
			features : "eigenfaces_2015_11_04",
		}
	}	

	var req_data = JSON.stringify(
	{
		requestVersion : 'v2',
		priority : 'high' ,
		input : [
			{
				type : 'image',
				encoding : 'hex',
				data : image,
			}
		],
		action : 
			{
				method : 'face_detection',
				options : options,
			}
	});

	var http_request_options = 
				{
				  host: 'as-imgproc2.dmzexdc01.bitdefender.biz'
				, port: 5000
				, method: 'POST'
				, path: '/'
				, headers:	{ 
							  'Content-Type' : 'application/json'
							, 'Content-Length' : Buffer.byteLength( req_data, 'utf8' )
							}
				};

		try{
			BDFD_request = http.request(http_request_options, function(response) 
				{
				var str = '';
				
				response.on( 'error', 
							 function ( chunk ) 
								{
								console.log( "["+(new Date()).toLocaleTimeString()+"] [Bitdefender] ERR IN REQUEST");
								
								}
							);
				
				response.on( 'data', // another chunk of data has been recieved, so append it to `str`
							 function ( chunk ) 
								{
								str += chunk;
								}
							);
				
				response.on( 'end', // the whole response has been recieved, so we just print it out here
							 function ( ) 
								{
									try
									{
										var str_json = JSON.parse( str );
										if(str_json.status == "ok")
											callback(null, str_json);	
										else
										{
											console.log( "["+(new Date()).toLocaleTimeString()+"] [Bitdefender] request.on('error'):", str_json.status_message );				

											setTimeout(function(){
												faceDetection(image, callback);
											},1000);
											return;
										}
									}
									catch(e)
									{
											console.log( "["+(new Date()).toLocaleTimeString()+"] [Bitdefender] request.on('error'):", e );				
											console.log(str)
											setTimeout(function(){
												faceDetection(image, callback);
											},1000);
											return;
									}																  
								}
							);
		  });
		  
		  BDFD_request.on('socket', function (socket) {
			socket.setTimeout(60* 60 * 1000);  
			socket.on('timeout', function() {
				console.log( "["+(new Date()).toLocaleTimeString()+"] [Bitdefender] Request TimeOut Reached" );
				process.exit(1);
			});
		});
		
		BDFD_request.on
					(	
						'error', 
						function ( error ) // Error handling here
						{
							console.log( "["+(new Date()).toLocaleTimeString()+"] [Bitdefender] request.on('error'):", error );				

							setTimeout(function(){
								faceDetection(image, callback);
							},1000);
							return;
						}
					);

		  BDFD_request.write(req_data);
		  BDFD_request.end();
		}
		
		catch(error){
			console.log( "["+(new Date()).toLocaleTimeString()+"] [Bitdefender] Catch Error: " + error);			

			setTimeout(function(){
							faceDetection(image, callback);
						},1000);
			return;
		}	
}

var faceCompare = function(resultArray, _cb)
{
	var req_data = JSON.stringify
				(
					{
						'requestVersion' : 'v2' ,
						'priority' : 'high' ,
						'input' : [
							{
								type : 'json',
								data : resultArray[0],
							},
							{
								type : 'json',
								data : resultArray[1],
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
				  host: 'as-imgproc2.dmzexdc01.bitdefender.biz'
				, port: 5000
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
													console.log( response_data )
													var response_json = JSON.parse( response_data );

													if(response_json.status == 'ok')
													{
														_cb(response_json)
													}
													else 
													{
														console.log( " BDF - NN error  http_request.on('error'):", response_json );
														faceCompare(resultArray, _cb);
													}												
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
									faceCompare(resultArray, _cb);
									}
							);
						}
				);
			
			http_request.on
				(	
					'error', 
					function ( error ) // Error handling here
						{
							console.log( " BDF - NN error  http_request.on('error'):", error );
							faceCompare(resultArray, _cb);
						}					
				);
				
			//---------------------------------------//
						
			http_request.write( req_data ); //This is the data we are posting, it needs to be a string or a buffer					
			http_request.end();
}

var processDetections = function(results, _cb)
{
	var output = 
	{
		serviceOutput : "",
		label : -1,
	}

	var projections = [];

	for(var i in results)
	{
		if(results[i].result && results[i].result.face_count == 1)
		{
			projections.push(results[i].result.fd[0].internal.features);
		}
		else
		{
			output.serviceOutput = "detection error";
			_cb(output);
			return;
		}
	}

	faceCompare(projections, function(result)
	{
		output.serviceOutput = result.result;

		var score = result.result[1];

		if( score > RECOGNITION_TRESHOLD )
			output.label = 1;
		else output.label = 0;

		_cb( output );
	})
}

processPair = function(i1Path, i2Path, _cb)
{
	// send to detection and projection extraction
	var img1 = fs.readFileSync(i1Path, 'hex');
	var img2 = fs.readFileSync(i2Path, 'hex');

	async.map([img1, img2], faceDetection, function(err, results)
	{
		// send to face compare
		processDetections(results, _cb);
	});
}


var bitDefenderFR = 
{
	name : 'bitDefenderFR',
	processPair : processPair,
}

module.exports = bitDefenderFR;
module.exports.faceDetection = faceDetection;