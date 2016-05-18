var fs = require('fs');
var http = require('https');
var async = require('async');

var faceDetection = function(image, callback)
{
	setTimeout(
	function()
		{
		var req_data_Azure = image;

		var http_request_options = 
		{
		  host: 'api.projectoxford.ai'
		, port: 443
		, method: 'POST'
		, path: '/face/v0/detections?subscription-key=60258017f09b4451a508224c361e5f1c&analyzesFaceLandmarks=false&analyzesAge=false&analyzesGender=false&analyzesHeadPose=false'
		, headers:  { 
				'Content-Type' : 'application/octet-stream'
			  , 'Content-Length' : req_data_Azure.length
			  }					  
		};	

			try {
				Azure_request = http.request(http_request_options, function(response) 
					{
					var str = '';
					
					response.on( 'error', 
								 function ( chunk ) 
									{
									console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] ERR IN REQUEST");
									process.exit(1);
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
									var str_json = JSON.parse( str );
										
									if( str_json.statusCode )
										{
										console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] Status: ", str_json, ' Retrying....');

										setTimeout(
													function()
														{
														faceDetection(image, callback);
														},
													1000
													);				
										return;
										}											

									console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] " + JSON.stringify(str_json));
									
									callback( null, str_json );			  
									}
								);
			  });
			  
			  Azure_request.on('socket', function (socket) {
				socket.setTimeout(60* 60 * 1000);  
				socket.on('timeout', function() {
					console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] Request TimeOut Reached" );
					process.exit(1);
					});
				});
			
			Azure_request.on
						(	
							'error', 
							function ( error ) // Error handling here
								{
								console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] request.on('error'):", error );
								setTimeout(
										function()
											{
											faceDetection(image, callback);
											},
										1000
										);
								return;
								}
						);

			  Azure_request.write(req_data_Azure);
			  Azure_request.end();
			}
			catch(error)
			{
			console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] Catch Error: " + error);
				setTimeout(
						function()
							{
							faceDetection(image, callback);
							},
						1000
						);
				return;
			}
		},
		1000 * 5
	);

}

var faceRecognitionReq = function(faceIds, callback)
{
	setTimeout(
	function()
		{
		req_data_Azure = 
			{
				  "faceId1":faceIds[0],
				  "faceId2":faceIds[1],
			}
		
		var http_request_options = 
			{
			  host: 'api.projectoxford.ai'
			, port: 443
			, method: 'POST'
			, path: '/face/v0/verifications?subscription-key=60258017f09b4451a508224c361e5f1c'
			, headers:  { 
					'Content-Type' : 'application/json',
				  }					  
			};			
		try {
			Azure_request = http.request(http_request_options, function(response) 
				{
				var str = '';
				
				response.on( 'error', 
							 function ( chunk ) 
								{
								console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] ERR IN REQUEST");
								process.exit(1);
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
								var str_json = JSON.parse( str );
									
								if( str_json.statusCode )
									{
									console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] Status: ", str_json, ' Retrying....');
									setTimeout(
												function()
													{
													faceRecognitionReq(faceIds, callback);
													},
												1000
												);				
									return;
									}											
							
								console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] " + JSON.stringify(str_json));
								
								callback( str_json );			  
								}
							);
		  });
		  
		  Azure_request.on('socket', function (socket) {
			socket.setTimeout(60* 60 * 1000);  
			socket.on('timeout', function() {
				console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] Request TimeOut Reached" );
				process.exit(1);
			});
		});
		
		Azure_request.on
					(	
						'error', 
						function ( error ) // Error handling here
							{
							console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] request.on('error'):", error );						
							setTimeout(
									function()
										{
										faceRecognitionReq(faceIds, callback);
										},
									1000
									);
							return;
							}
					);

		  Azure_request.write(JSON.stringify(req_data_Azure));
		  Azure_request.end();
		}				
	catch(error)
		{
		console.log( "["+(new Date()).toLocaleTimeString()+"] [Microsoft] Catch Error: " + error);	
		setTimeout(
				function()
					{
					faceRecognitionReq(faceIds, callback);
					},
				1000
				);
		return;
		}
	},
	1000 * 5
	);
}

var processDetections = function(results, _cb)
{
	var output = 
	{
		serviceOutput : "",
		label : -1,
	}	

	var faceIds = [];

	for(var i in results)
	{
		if( results[i].length == 1 && results[i][0].faceId )
		{
			faceIds.push( results[i][0].faceId );
		}
		else
		{
			console.log("["+(new Date()).toLocaleTimeString()+"] [Microsoft] " + "detection error: " + JSON.stringify(results[i]))
			output.serviceOutput = "detection error";
			_cb(output);
			return;
		}
	}

	faceRecognitionReq(faceIds, function(result)
	{
		output.serviceOutput = result;		

		if( result.isIdentical == true )
			output.label = 1;
		else output.label = 0;

		_cb( output );
	})
}

processPair = function(i1Path, i2Path, _cb)
{
	// send to detection and projection extraction
	var img1 = fs.readFileSync(i1Path);
	var img2 = fs.readFileSync(i2Path);

	async.map([img1, img2], faceDetection, function(err, results)
	{
		// send to face compare
		processDetections(results, _cb);
	});
}

var microsoftOxfordFR = 
{
	name : 'microsoftOxfordFR',
	processPair : processPair,
}

module.exports = microsoftOxfordFR
