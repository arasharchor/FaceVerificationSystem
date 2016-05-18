var fs = require('fs')
var config = require('../config');
var faceDetection = require('./services/bitDefenderFR').faceDetection;
var async = require('async');
var crypto = require('crypto');

/************************************************************************/

var METADATA_LOCK = false;
var METADATA_LOCK_RETRY_TIME = 500;

/***********************************************************************/

// readin metadata
var loadInputStats = function () {

	var metadataDir = fs.readdirSync(config.metadataPath);
	var stats = {};

	for(var i in metadataDir)
	{
		var testName = metadataDir[i];
		var test_json = JSON.parse(fs.readFileSync(config.metadataPath + testName));

		testName = testName.replace(".js", "");

		stats[testName] = test_json.stats;
	}	

	return stats;
}

var validateFaceDetecion = function(results)
{
	for(var i in results)
	{
		if(!results[i].result) 
		{
			return "No result found for face " + i;
		}
		else if( results[i].result.face_count != 1 )
		{
			return "Wrong face count for image " + (parseInt(i) + 1) + " : " + results[i].result.face_count + " faces";
		}
	}

	return false;
}

var writeImages = function(image1_name, image2_name, image1_hex, image2_hex, testName)
{
	var writePath = config.testsPath + testName + "/";
	fs.writeFileSync(writePath + image1_name, new Buffer(image1_hex, 'hex'));
	fs.writeFileSync(writePath + image2_name, new Buffer(image2_hex, 'hex'));
}

var writeMetadata = function(image1_name, image2_name, testName, label)
{
	if( true == METADATA_LOCK )
	{
		setTimeout(function()
		{
			writeMetadata(image1_name, image2_name, testName, label);
		}, METADATA_LOCK_RETRY_TIME)
	}
	else
	{
		METADATA_LOCK = true;
		var metadata = JSON.parse(fs.readFileSync(config.metadataPath + testName + ".js"));
		var newPair = 
		{
			"i1" : image1_name ,
			"i2" : image2_name ,
			"label" : parseInt(label),
		}

		metadata.pairs.push(newPair);
		metadata.stats.total++;

		if(label == 1)
		{
			metadata.stats.positive++;
		}
		else metadata.stats.negative++;

		//console.log(metadata);

		fs.writeFileSync(config.metadataPath + testName + ".js", JSON.stringify(metadata, null, 4));
		METADATA_LOCK = false;

		return 
	}
}

var processInputPair = function(image1_b64, image2_b64, label, testName, res)
{
	var image1_hex = new Buffer(image1_b64, 'base64').toString('hex');
	var image2_hex = new Buffer(image2_b64, 'base64').toString('hex');

	// check if images contain exacly one face
	async.map([image1_hex, image2_hex], faceDetection, function(err, results)
	{		
		var validationMessage = validateFaceDetecion(results);
		if(validationMessage)
		{
			res.send(validationMessage);
		}
		else
		{
			// get md5 for each image
			var image1_name = crypto.createHash('md5').update(image1_hex).digest("hex") + ".png";
			var image2_name = crypto.createHash('md5').update(image2_hex).digest("hex") + ".png";
			
			// write images to location
			writeImages(image1_name, image2_name, image1_hex, image2_hex, testName);

			// add data to metadata
			writeMetadata(image1_name, image2_name, testName, label);
			res.send('Succes!!!')
		}
	});

}

module.exports.loadInputStats = loadInputStats;
module.exports.processInputPair = processInputPair;