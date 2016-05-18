var fs = require('fs');
var config = require('../config');

var getProgress = function(testMaster)
{
	var servicesDone = {};

	var progress = 
	{
		total : 0,
		services : {},
		time : {},
	}

	var grandTotal = testMaster.info.services.length * testMaster.info.total;

	var done = 0;

	for(var i in testMaster.tests)
	{
		for(var k in testMaster.tests[i].stats)
		{
			done += testMaster.tests[i].stats[k].done;

			if(servicesDone[k])
			{
				servicesDone[k] += testMaster.tests[i].stats[k].done;
			} 
			else
			{
				servicesDone[k] = testMaster.tests[i].stats[k].done;
			}
		}
	} 
	
	progress.total = parseFloat(100 * done/grandTotal).toFixed(2);

	for(var i in servicesDone)
	{
		progress.services[i] = parseFloat(100 * servicesDone[i]/testMaster.info.total).toFixed(2);

		progress.time[i] = testMaster.globals[i].avgTime;
	}

	return progress;
}

var readOutputsList = function () {
var
	 output_stats = {};

	var ouput_files = fs.readdirSync(config.outputPath);

	// read in test infos
	for(var i in ouput_files)
	{
		var testMaster = JSON.parse(fs.readFileSync(config.outputPath + ouput_files[i]));
		testMaster.info.progress = getProgress(testMaster).total;

		var info = testMaster.info;
		output_stats[ouput_files[i]] = info;
	}

	return output_stats;
} 

module.exports.readOutputsList = readOutputsList;
module.exports.getProgress = getProgress;