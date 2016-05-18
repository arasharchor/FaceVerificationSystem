// load ouput file *********************************************************************************************

var updateGlobals = function(globals, fileName, total, info)
{
	var timeStamp = fileName.split('.')[0].split('_').pop();
	var date = (new Date(parseInt(timeStamp))).toGMTString()

	$('#testGlobals').empty();

	//$('#testGlobals').append("<h5 style='margin-left:40%'><strong>" + fileName + "</strong></h5>");

	$('#testGlobals').append("<h5><strong>	Tests: </strong>" + info.tests.toString() + "</h5>");
	$('#testGlobals').append("<h5><strong>	Services: </strong>" + info.services.toString() + "</h5>");
	$('#testGlobals').append("<h5><strong>	Date: </strong>" + date + "</h5>");
	$('#testGlobals').append("<h5><strong>	Progress: </strong>" + info.progress.total + " %</h5>");

	/*var testMasterTable = "<table class='table' id='ouputFilesTable'><thead>" +
	                	  "<tr><th>ServiceName</th>" +
	                	  "<th>F1 Score</th>" +
	                      "<th>Detection Rate</th>" + 
	                      "<th>Precision</th>" + 
	                      "<th>FN</th>" + 
	                      "<th>FP</th>" + 
	                      "<th>TN</th>" +
	                      "<th>TP</th>" +
	                      "<th>Progress</th>" +
	                      "<th>Avg Time</th>";
	testMasterTable += "</tr></thead><tbody>";
	*/

	var testMasterTable = "<table class='table' id='ouputFilesTable'><thead>" +
	                	  "<tr>" + 
	                	  "<th>ServiceName</th>" +
	                	  "<th>ACC</th>" +
	                      "<th>F1 score</th>" + 
	                      "<th>TPR</th>" + 
	                      "<th>TNR</th>" + 
	                      "<th>PPV</th>" + 
	                      "<th>NPV</th>" +
	                      "<th>FN</th>" +
	                      "<th>FP</th>" +
	                      "<th>TN</th>" +
	                      "<th>TP</th>" +
	                      "<th>Progress</th>" +
	                      "<th>Avg Time</th>"
	testMasterTable += "</tr></thead><tbody>";

	console.log(testMasterTable);


	for(var i in globals)
	{
		/*
		var detection = parseFloat(((100 * globals[i].TP) / (globals[i].TP + globals[i].FN)).toFixed(2));
		var precision = parseFloat(((100 * globals[i].TN) / (globals[i].TN + globals[i].FP)).toFixed(2));
		var f1score = parseFloat(((2 * precision * detection) / (precision + detection)).toFixed(2));
		*/
		
		var fn = globals[i].FN
		var fp = globals[i].FP
		var tn = globals[i].TN
		var tp = globals[i].TP

		var tpr = parseFloat(100 * tp / (tp + fn)).toFixed(2);
		var spc = parseFloat(100 * tn / (tn + fp)).toFixed(2);
		var ppv = parseFloat(100 * tp / (tp + fp)).toFixed(2);
		var npv = parseFloat(100 * tn / (tn + fn)).toFixed(2);
		var acc = parseFloat(100 * (tp + tn) / (tp + fp + fn + tn)).toFixed(2);
		var f1s = parseFloat(100 * 2 * tp / (2 * tp  + fp + fn)).toFixed(2);		

		testMasterTable += "<tr>";
		testMasterTable += "<td>" + i + "</td>";
		testMasterTable += "<td>" + acc + " %</td>";
		testMasterTable += "<td>" + f1s + " %</td>";
		testMasterTable += "<td>" + tpr + " %</td>";
		testMasterTable += "<td>" + spc + " %</td>";
		testMasterTable += "<td>" + ppv + " %</td>";
		testMasterTable += "<td>" + npv + " %</td>";

		testMasterTable += "<td>" + fn + "</td>";
		testMasterTable += "<td>" + fp + "</td>";
		testMasterTable += "<td>" + tn + "</td>";
		testMasterTable += "<td>" + tp + "</td>";

		/*
		testMasterTable += "<td>" + f1score + " %</td>";
		testMasterTable += "<td>" + detection + " %</td>";
		testMasterTable += "<td>" + precision + " %</td>";
		
		testMasterTable += "<td>" + globals[i].FN + "</td>";
		testMasterTable += "<td>" + globals[i].FP + "</td>";
		testMasterTable += "<td>" + globals[i].TN + "</td>";
		testMasterTable += "<td>" + globals[i].TP + "</td>";
		*/

		testMasterTable += "<td>" + info.progress.services[i] + " %</td>";
		testMasterTable += "<td>" + info.progress.time[i] + " sec/pair</td>";
		testMasterTable += "</tr>";
	}

	console.log(testMasterTable);

	testMasterTable += "</tbody></table>";

	$('#testGlobals').append(testMasterTable)
}


var loadOuputFile = function(fileName)
{
	// get file
	$.post('/getOuputFile', {fileName : fileName}, function(testMaster)
	{
		console.log(testMaster);
		// display general stats 
		updateGlobals(testMaster.globals, fileName, testMaster.info.total, testMaster.info);
	})	

	// display detailed stats
}

//**************************************************************************************************************

// display all output files to *********************************************************************************

var renderOuptuFilesList = function () {

	var ouputFilesTable = "<table class='table' id='ouputFilesTable'><thead>" +
	                	"<tr><th>FileName</th>" +
	                    "<th>Date</th>" + 
	                    "<th>Tests</th>" + 
	                    "<th>Services</th>" +
	                    "<th>Progress</th>" +
	                    "<th>Action</th></tr></thead><tbody>";

	for(var i in server_data.outputFiles)
	{
		var id = i.split('.')[0];

		ouputFilesTable += "<tr><td>" + i + "</td>";

		var timeStamp = id.split('_').pop();

		var date = (new Date(parseInt(timeStamp))).toGMTString()

		ouputFilesTable += "<td>" + date + "</td>";
		ouputFilesTable += "<td>" + server_data.outputFiles[i].tests.toString() + "</td>";
		ouputFilesTable += "<td>" + server_data.outputFiles[i].services.toString() + "</td>";
		ouputFilesTable += "<td id='progress_" + id + "'>" + server_data.outputFiles[i].progress + " %</td>";

	    ouputFilesTable += "<td><a href='#' class='btn btn-sm btn-success' id='" + id + "'>Load</a></td></tr>";
	}

	ouputFilesTable += "</tbody></table>";

	$('#oLoadArea').append(ouputFilesTable);

	for(var i in server_data.outputFiles)
	{
		if(server_data.outputFiles[i].progress != 100)
		{
			updateProgress(i);
		}

		var id = i.split('.')[0];

		$('#' + id).click(function()
		{
			var fileName = $(this).attr('id') + ".json";
			loadOuputFile(fileName);
		})
	}

}

//***************************************************************************************************************
renderOuptuFilesList();