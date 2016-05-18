
var POLLING_INTERVAL = 1000 * 5;

var addFileRow = function(testMaster)
{
	var testName = testMaster.testName;

	var id = testName.split('.')[0];

	var tr = "<tr><td>" + testName + "</td>";

	var timeStamp = id.split('_').pop();
	var date = (new Date(parseInt(timeStamp))).toGMTString();

	tr += "<td>" + date + "</td>";
	tr += "<td>" + testMaster.testStats.tests.toString() + "</td>";
	tr += "<td>" + testMaster.testStats.services.toString() + "</td>";
	tr += "<td id='progress_" + id + "'>0 %</td>";

    tr += "<td><a href='#' class='btn btn-sm btn-success' id='" + id + "'>Load</a></td></tr>";

    $('#ouputFilesTable tr:last').after(tr);

	$('#' + id).click(function()
	{
		var fileName = $(this).attr('id') + ".json";
		loadOuputFile(fileName);
	})
}

var updateProgress = function(testId)
{
	var id = testId.split('.')[0];

	var progressInterval = setInterval(function()
	{
		$.post('./getTestProgress', {testName : testId}, function(progress)
		{
			$('#progress_' + id).html(progress + " %")
			if(progress == 100)
			{				
				clearInterval(progressInterval)
			}
		})
	}, POLLING_INTERVAL);
}

var runTest = function(testsArray)
{
	$.post('/runTest', { "tests" : testsArray }, function(response)
	{

		console.log(response);

		if(response != 'false')
		{	
			alert('Test started !!!');

			//add row to ouput files table
			addFileRow(response);

			updateProgress(response.testName);
		}
		else
		{
			alert('Max test instances reached, please try again later');
		}
	})
}

$('#runTestBtn').click(function() {
	var runTestArray = [];
	
	// get tests
	$('.test_input_class').each(function () {
	       var sThisVal = (this.checked ? $(this).val() : "");
	       if(sThisVal)
	       {
	       		runTestArray.push(sThisVal)
	       }	       
	  });
	
	if(!runTestArray.length)
	{
		alert('Please check at least one test to be run !!!')
	}
	else
	{
		runTest(runTestArray);		
	}
})