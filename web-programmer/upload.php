<html>
<body style="background-color: #000000; color: #00FF00;">

<?php

function parse_memory_set($fname, $dirname, $outfname) {

    $nummaps = 32;
    $mapsize = $nummaps * 65536;
    $memorymaps = array();
    for ($i = 0; $i < $mapsize; $i++)
    {
        $memorymaps[$i] = 0;
    }

    printf("s: %d<br>\n", sizeof($memorymaps));
    
    // parse each line
    foreach (file($fname) as $line) {
        if (strlen($line) < 2) {
            continue;
        }

        // skip comments
        if ($line[0] == '#' || ($line[0] == '/' && $line[1]) == '/') {
            continue;
        }

        $parts = explode(",", $line);
        print_r($parts);
        printf("<br>");

        // get index
        $index = $parts[0];
        // get address
        $address = base_convert($parts[2], 16, 10);
        printf("index: %d, address %s<br>\n", $index, $address);

        $rom_fname = sprintf("%s/%s", $dirname, $parts[1]);
        if (file_exists($rom_fname))
        {
            $rom_contents = file_get_contents($rom_fname);
            printf("size of %s: %d<br>\n", $rom_fname, strlen($rom_contents));

            // copy contents into memory map
            $start_addr = ($index * 65536) + $address;
            printf("copying to %d<br>\n", $start_addr);

            // now copy the data
            for ($b = 0; $b < strlen($rom_contents); $b++)
            {
                //printf("writing to %d, content %d %s %X<br>\n", $b, $rom_contents[$b]);
                $a = unpack("Ccontents", $rom_contents[$b]);
                $memorymaps[$start_addr + $b] = $a["contents"];
                //printf("got %d %d\n", $b, $memorymaps[$start_addr + $b]);
            }
        }
    }

    printf("s: %d<br>\n", sizeof($memorymaps));
    // now write entire memory map contents to file
    $fp = fopen($outfname, "wb");
    for ($b = 0; $b < $mapsize; $b++)
    {
        printf("%d %X<br>\n", $b, $memorymaps[$b]);
        fwrite($fp, pack("C*", $memorymaps[$b]));
    }
    fclose($fp);
}


//if (isset($_POST["submit"])) {
if (true) {

    $r = rand();
    $target = sprintf("%d.zip", $r);
    $dirname = "test";

    $uploadok = 1;

    /*
    $filetype = strtolower(pathinfo(basename($_FILES["firmware_zip"]["name"]),PATHINFO_EXTENSION));
    if ($filetype != "zip") {
        echo "error, only zip files allowed.";
        $uploadok = 0;
    }

    if ($uploadok == 1) {
        if (move_uploaded_file($_FILES["firmware_zip"]["tmp_name"], $target)) {
            echo "The file ". htmlspecialchars( basename( $_FILES["firmware_zip"]["name"])). " has been uploaded.";
        } else {
            echo "Problem uploading file.";
            die();
        }
    }

    // extract the zip file
    $zip = new ZipArchive;
    $res = $zip->open($target);
    if ($res === TRUE) {
        $dirname = sprintf("%d", $r);
        mkdir($dirname);

        $zip->extractTo($dirname);
        $zip->close();
    } else {
        echo 'failed to open zip file.';
        unlink($target);
        die();
    }

    // remove zip file
    unlink($target);
    */

    // find enable table
    $enable_table = "";
    $s = sprintf("%s/enable_table*.csv", $dirname);
    $f = glob($s);

    if (sizeof($f) == 1) {
        $enable_table = $f[0];
        printf("found enable_table: %s\n", $enable_table);
    } else {
        printf("could not find enable table!\n");
        die();
    }

    // find memory set
    $s = sprintf("%s/memory_set*.csv", $dirname);
    $f = glob($s);

    if (sizeof($f) == 1) {
        $memory_set = $f[0];
        printf("found memory set: %s\n", $memory_set);
    } else {
        printf("could not find memory set!\n");
        die();
    }

    // start with memory set
    parse_memory_set($memory_set, $dirname, "test/memoryset.bin");
    // done

} else { 
    echo '
<form action="upload.php" method="post" enctype="multipart/form-data">
Choose file to upload:
<input type="file" id="firmware_zip" name="firmware_zip" accept=".zip">
<input type="submit" value="Upload" name="submit">
</form>
    ';
}
?>

</body>
</html>