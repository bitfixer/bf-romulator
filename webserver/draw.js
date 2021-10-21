window.onload = function() {

var canvas = document.getElementById("myCanvas");
var ctx = canvas.getContext("2d");

var width = canvas.width;
var height = canvas.height;

var imagedata = ctx.createImageData(width, height);

var bitmapArray = new Uint8Array(64000);
var characterRom;

function loadRom()
{
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "romulator.rom", true);
    xhttp.responseType = 'arraybuffer';

    xhttp.onload = function(e) {
        var arrayBuffer = xhttp.response;
        if (arrayBuffer) {
            characterRom = new Uint8Array(arrayBuffer);
            console.log(characterRom.byteLength);
            setTimeout(createImage, 0);
        }
    }
    xhttp.send(null);
}

function createImage(offset)
{
    // retrieve data
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "romulator.vram", true);
    xhttp.responseType = 'arraybuffer';

    xhttp.onload = function(e) {
        if (this.status == 200) {
            var arrayBuffer = xhttp.response;

            if (arrayBuffer) {
                var byteArray = new Uint8Array(arrayBuffer);

                console.log("vram ", byteArray.byteLength);
                romulatorVramToBitmap(
                    byteArray,
                    characterRom,
                    25,
                    40,
                    8,
                    8,
                    bitmapArray);

                console.log("bitmap ", bitmapArray.byteLength);

                for (var i = 0; i < bitmapArray.byteLength; i++)
                {
                    var pixelindex = i * 4;

                    var val = bitmapArray[i];
                    if (val > 0) {
                        val = 255;
                    }

                    imagedata.data[pixelindex] = val;
                    imagedata.data[pixelindex+1] = val;
                    imagedata.data[pixelindex+2] = val;
                    imagedata.data[pixelindex+3] = 255;
                }

                ctx.putImageData(imagedata, 0, 0);
                setTimeout(createImage, 30, 0);
                /*
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
                */
            }
            else
            {
                alert("no buffer");
            }
        }
    }

    xhttp.send(null);
}

loadRom();

}