function drawCursor(
    row,
    col,
    bitmap,
    imageWidth)
{
    var xstart = col * 8;
    var ystart = row * 8;

    var bitmapIndex = (ystart * imageWidth) + xstart;
    for (var yy = 0; yy < 8; yy++)
    {
        for (var xx = 0; xx < 8; xx++)
        {
            var byte = bitmap[bitmapIndex+xx];
            if (byte == 0)
            {
                byte = 255;
            }
            else
            {
                byte = 0;
            }

            bitmap[bitmapIndex+xx] = byte;
        }

        bitmapIndex += imageWidth;
    }
}


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
                bitmap[pixelIndex] = 255;
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
            var x = col * charWidth;
            var y = row * charHeight;
            var character = vram[charIndex++];
            drawCharacter(character, x, y, bitmap, characterRom, imageWidth);
        }
    }
}