var express = require('express');
var router = express.Router();
var fs = require('fs');
/******************************************************/
var inputFunctions = require('../lib/inputFunctions');
var outputFunctions = require('../lib/outputFunctions');
var testScript = require('../testScript');
var config = require('../config');
/******************************************************/
var data = {};
var MAX_TEST_INSTANCES = 1;
var NR_TEST_INSTANCES  = -1;

var testMasters = [];
/******************************************************/

// check if tests are done
setInterval(function()
{
	for(var i in testMasters)
	{
		if(testMasters[i].done == true)
		{
			NR_TEST_INSTANCES--;
		}
	}
}, 1000 * 2);


/* GET home page. */
router.get('/', function(req, res) {
	data.inputStats = inputFunctions.loadInputStats();
	data.outputFiles = outputFunctions.readOutputsList();

    res.render('index', { title: 'Face Recognition Test' , data : data})
});

router.post('/inputPair', function(req, res)
{
	//console.log(req.body);
	var image1_b64 = req.body.image1.substring(req.body.image1.indexOf(',') + 1);
	var image2_b64 = req.body.image2.substring(req.body.image2.indexOf(',') + 1);

	var label = req.body.label;
	var test_name = req.body.test;

	inputFunctions.processInputPair(image1_b64, image2_b64, label, test_name, res);	
})

router.post('/runTest', function(req, res)
{
	// prepare config - TO DO
	var config_temp = JSON.parse(JSON.stringify(config));

	var tests = req.body['tests[]'];

	if(Array.isArray(tests))
	{
		config_temp.tests = tests;
	}
	else
	{
		config_temp.tests = [ tests ];
	}

	config_temp.outputName = 'test_result_' + (new Date()).getTime() + '.json';

	if( NR_TEST_INSTANCES + 1 < MAX_TEST_INSTANCES)
	{
		NR_TEST_INSTANCES++;

		testMasters[NR_TEST_INSTANCES] = testScript(config_temp);
		
		var testStats = testMasters[NR_TEST_INSTANCES].info;

		res.send({ testName: config_temp.outputName, testStats : testStats});
	}
	else 
	{
		res.send("false");
	}
});


router.post('/getOuputFile', function(req, res)
{
	var fileName = req.body.fileName;

	var outfile = JSON.parse(fs.readFileSync(config.outputPath + fileName));

	outfile.info.progress = outputFunctions.getProgress(outfile);

	res.json(outfile);
})

router.post('/getTestProgress', function(req, res)
{
	var fileName = req.body.testName;

	var test = JSON.parse(fs.readFileSync(config.outputPath + fileName));

	var progress = outputFunctions.getProgress(test).total;

	res.send(progress + "");
})

module.exports = router;


/** tips

**/
