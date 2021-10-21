function drawCharacter(
    character,
    x,
    y,
    bitmap,
    characterRom,
    imageWidth)
{
    for (var yy = 0; yy < 8; yy++)
    {
        var byteIndex = (character * 8) + yy;
        var byte = characterRom[byteIndex];
        //console.log("rom ", byte);
        for (var xx = 0; xx < 8; xx++)
        {
            var bitIndex = 7 - xx;
            var pixelIndex = ((y + yy) * imageWidth) + (x + xx);

            if ((byte & (1 << bitIndex)) == 0)
            {
                bitmap[pixelIndex] = 0;
            }
            else
            {
                //console.log(xx, yy, pixelIndex);
                bitmap[pixelIndex] = 1;
            }
        }
    }
}


function romulatorVramToBitmap(
    vram,
    characterRom,
    rows,
    columns,
    charWidth,
    charHeight,
    bitmap)
{
    var imageWidth = columns * charWidth;
    var imageHeight = rows * charHeight;

    var charIndex = 0;
    for (var row = 0; row < rows; row++)
    {
        for (var col = 0; col < columns; col++)
        {
            var x = col * 8;
            var y = row * 8;
            var character = vram[charIndex++];
            //console.log("char ", row, col, character);
            drawCharacter(character, x, y, bitmap, characterRom, imageWidth);
        }
    }
}