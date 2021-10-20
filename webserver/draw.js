window.onload = function() {

var canvas = document.getElementById("myCanvas");
var ctx = canvas.getContext("2d");

var width = canvas.width;
var height = canvas.height;

var imagedata = ctx.createImageData(width, height);

function createImage(offset)
{
    // retrieve data
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "http://192.168.0.102:10000/romulator.bin", true);
    xhttp.responseType = 'arraybuffer';

    xhttp.onload = function(e) {
        if (this.status == 200) {
            var arrayBuffer = xhttp.response;

            if (arrayBuffer) {
                var byteArray = new Uint8Array(arrayBuffer);
                for (var i = 0; i < byteArray.byteLength; i++) 
                {
                    var pixelindex = i * 4;

                    var val = byteArray[i];
                    if (val > 0) {
                        val = 255;
                    }

                    imagedata.data[pixelindex] = val;
                    imagedata.data[pixelindex+1] = val;
                    imagedata.data[pixelindex+2] = val;
                    imagedata.data[pixelindex+3] = 255;
                }
                
                ctx.putImageData(imagedata, 0, 0);
            }
            else
            {
                alert("no buffer");
            }
        }
    }

    xhttp.send(null);
}

/*
function main(tframe)
{
    window.requestAnimationFrame(main);

    createImage(Math.floor(tframe / 10));
    ctx.putImageData(imagedata, 0, 0);
}
*/

//main(0);

createImage(0);

}