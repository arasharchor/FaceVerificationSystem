console.log(server_data);

// globals *******************************************************************
var INPUT_IMAGES = 
{
    image1 : "",
    image2 : "",
    label  : null,
    test   : null,
}

// colors
var COLORS = 
{
    default : 'rgb(235, 235, 235)',
    red     : 'rgb(220, 10, 10)',
}

// image form handler ********************************************************
function readURL_1(input) {
    if (input.files && input.files[0]) {
        var reader = new FileReader();
        
        reader.onload = function (e) {
            $('#inputImage1').attr('src', e.target.result);
            INPUT_IMAGES.image1 = e.target.result;            
        }
        
        reader.readAsDataURL(input.files[0]);
    }
}

$("#image1form").change(function(){
    readURL_1(this);
});

function readURL_2(input) {
    if (input.files && input.files[0]) {
        var reader = new FileReader();
        
        reader.onload = function (e) {
            $('#inputImage2').attr('src', e.target.result);
            INPUT_IMAGES.image2 = e.target.result; 
        }
        
        reader.readAsDataURL(input.files[0]);
    }
}

$("#image2form").change(function(){
    readURL_2(this);
});

//********************************************************************************

// display input stats ***********************************************************
var table = "<table class='table' id='statsTable'><thead>" +
                "<tr><th>Action</th>" +
                    "<th>TestName</th>" + 
                    "<th>Total</th>" + 
                    "<th>Positive</th>" +
                    "<th>Negative</th>" + 
                    "<th></th></tr></thead><tbody>";

var renderTestTable = function(inputStats)
{
    $('#inputStatsTable').empty();
    for(var i in server_data.inputStats)
    {
        table += "<tr><td><a href='#' class='btn btn-sm btn-success' id='" + i + "'>Add pair</a></td>";
        table += "<td>" + i + "</td>";
        table += "<td>" + server_data.inputStats[i].total + "</td>";
        table += "<td>" + server_data.inputStats[i].positive + "</td>";
        table += "<td>" + server_data.inputStats[i].negative + "</td>";
        table += '<td><input class="test_input_class" type="checkbox" value="' + i + '"></td></tr>';
    }

    table += '<tr><td><a href="#" class="btn btn-success" id="runTestBtn" colspan="5">Run Test</a></td></tr>';
    //table += '<td></td><td></td><td></td><td></td></tr>';


    table += "</tbody></table>";

    $('#inputStatsTable').append(table);

    // set event for every button 
    for(var i in server_data.inputStats)
    {
        $('#' + i).click(function()
        {
            var test_name = $(this).attr('id');
            INPUT_IMAGES.test = test_name;
            submitInputData();
        })
    }
}

if(server_data.inputStats)
{
    renderTestTable(server_data.inputStats);
}

// dispaly label
var positiveLable = '<a href="#" class="btn btn-default" id="plabelInput">Positive</a>'
var negativeLable = '<a href="#" class="btn btn-default" id="nlabelInput">Negative</a>'

$('#inputStats').append("<h5>Label</h5>");
$('#inputStats').append(positiveLable);
$('#inputStats').append(negativeLable);

$('#plabelInput').click(function(){
    if($(this).css('background-color') == COLORS.default)
    {
        INPUT_IMAGES.label = 1;
        $(this).css('background-color', COLORS.red);
        $('#nlabelInput').css('background-color', COLORS.default);
    }
    else
    {
        INPUT_IMAGES.label = null;
        $(this).css('background-color', COLORS.default);
    }   
});

$('#nlabelInput').click(function(){
    if($(this).css('background-color') == COLORS.default)
    {
        INPUT_IMAGES.label = 0;
        $(this).css('background-color', COLORS.red);
        $('#plabelInput').css('background-color', COLORS.default);
    }
    else
    {
        INPUT_IMAGES.label = null;
        $(this).css('background-color', COLORS.default);
    }    
});

/********************************************************************************/
// ajax functions

var submitInputData = function()
{
    if(!INPUT_IMAGES.image1 || !INPUT_IMAGES.image2)
    {
        alert('Please upload photos');
        return;
    }

    if(INPUT_IMAGES.label == null)
    {
        alert('Please select label')
        return;
    }

    lockScreen();

    $.post('/inputPair', INPUT_IMAGES, function(response)
    {
        unlockScreen();
        if(response == "Succes!!!")
        {
           alert(response);
           location.reload(); 
        }
        else
        {
            alert(response);
        }
        
    })
}


//*******************************************************************************/
// client display functions
var lockScreen = function()
{
    $("#lockscreen").css("width", "100%");
    $("#lockscreen").css("height", "100%");
    $("#lockscreen").css("z-index", "1000");
}

var unlockScreen = function()
{
    $("#lockscreen").css("width", "0");
    $("#lockscreen").css("height", "0");
    $("#lockscreen").css("z-index", "0");
}
/********************************************************************************/