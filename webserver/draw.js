window.onload = function() {

var canvas = document.getElementById("myCanvas");
var ctx = canvas.getContext("2d");

var imagedata = ctx.createImageData(canvas.width, canvas.height);
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
                                var pixelIndex = ((yy * canvas.width) + xx) * 4;
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
                setTimeout(createImage, 0);
            }
            else
            {
                alert("no buffer");
            }
        }
    }

    xhttp.send(null);
}

function resize()
{
    var width = document.documentElement.clientWidth;
    var height = document.documentElement.clientHeight;

    // check scaling for the canvas
    var newWidth;
    var newHeight;
    if (width >= 1280 && height >= 800)
    {
        newWidth = 1280;
        newHeight = 800;
        scale = 4;
    }
    else if (width >= 640 && height >= 400)
    {
        newWidth = 640;
        newHeight = 400;
        scale = 2;
    }
    else
    {
        newWidth = 320;
        newHeight = 200;
        scale = 1;
    }

    if (newWidth != canvas.width || newHeight != canvas.height)
    {
        canvas.width = newWidth;
        canvas.height = newHeight;
        imagedata = ctx.createImageData(canvas.width, canvas.height);
    }

    var xMargin = width > canvas.width ? (width - canvas.width) / 2 : 0;
    var yMargin = height > canvas.height ? (height - canvas.height) / 2 : 0;

    canvas.style.left = xMargin + "px";
    canvas.style.top = yMargin + "px";
}

window.addEventListener('resize', resize);
loadRom();
resize();

}