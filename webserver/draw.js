window.onload = function() {

var canvas = document.getElementById("myCanvas");
var ctx = canvas.getContext("2d");

var width = canvas.width;
var height = canvas.height;

var imagedata = ctx.createImageData(width, height);

var bitmapArray = new Uint8Array(64000);
var characterRom;
var scale = 2;

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

function createImage()
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

                romulatorVramToBitmap(
                    byteArray,
                    characterRom,
                    25,
                    40,
                    8,
                    8,
                    bitmapArray);

                var cursorStatus = byteArray[0x04AA];
                console.log("cursor ", cursorStatus);

                if (cursorStatus == 1)
                {
                    var cursorRow = byteArray[0x04D8];
                    var cursorCol = byteArray[0x04C6];
                    console.log("row ", cursorRow, " col ", cursorCol);
                    drawCursor(cursorRow, cursorCol, bitmapArray, 320);
                }

                var i = 0;
                var ydest = 0;
                for (var y = 0; y < 200; y++)
                {
                    var xdest = 0;
                    for (var x = 0; x < 320; x++)
                    {
                        var val = bitmapArray[i];
                        for (yy = ydest; yy < ydest + scale; yy++)
                        {
                            for (xx = xdest; xx < xdest + scale; xx++)
                            {
                                var pixelIndex = ((yy * width) + xx) * 4;
                                imagedata.data[pixelIndex] = val;
                                imagedata.data[pixelIndex+1] = val;
                                imagedata.data[pixelIndex+2] = val;
                                imagedata.data[pixelIndex+3] = 255;
                            }
                        }

                        i++;
                        xdest += scale;
                    }

                    ydest += scale;
                }


                ctx.putImageData(imagedata, 0, 0);
                //setTimeout(createImage, 30, 0);
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