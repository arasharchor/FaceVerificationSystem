
var config = 
{
	testsPath : __dirname + '/tests/',
	tests : ['test_2', 'orl_test_1', 'lfw_test_small'],
	services : [],
	outputPath : __dirname + '/outputs/',
	metadataPath : __dirname + '/tests/metadata/',
	//services : ['microsoftOxfordFR', 'bitDefenderFR',  'faceVerification'], //, 'microsoftOxfordFR'
	services : ['faceVerification'], 
	servicesPath : __dirname + '/lib/services/',
	outputName : 'test_result_' + (new Date()).getTime() + '.json',
}

module.exports = config;