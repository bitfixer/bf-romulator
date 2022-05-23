<?php

function deleteDirectory($path) {
    $files = array_diff(scandir($path), array('.', '..'));
    foreach ($files as $file) {
        unlink("$path/$file");
    }
    return rmdir($path);
}

function failOperation($dirname) {
    deleteDirectory($dirname);
    die();
}

function parse_memory_set($fname, $dirname) {
    $nummaps = 32;
    $mapsize = $nummaps * 65536;
    $memorymaps = str_repeat(pack("C", "0"), $mapsize);
    // parse each line
    $linenum = 0;
    foreach (file($fname) as $line) {
        $linenum = $linenum + 1;
        if (strlen($line) < 2) {
            continue;
        }

        // skip comments
        if ($line[0] == '#' || ($line[0] == '/' && $line[1]) == '/') {
            continue;
        }

        // get index
        $parts = explode(",", $line);
        if (sizeof($parts) != 3)
        {
            printf("error in %s line %d: should be %d fields, found %d<br>\n", $fname, $linenum, 3, sizeof($parts));
            failOperation($dirname);
        }

        $index = $parts[0];
        // get address
        $address = base_convert($parts[2], 16, 10);
        $rom_fname = sprintf("%s/%s", $dirname, $parts[1]);
        if (file_exists($rom_fname))
        {
            $rom_contents = file_get_contents($rom_fname);
            if ((strlen($rom_contents) + $address) > 65536) {
                printf("error in %s line %d: rom %s too big (size %d) to fit at address %X<br>\n", 
                    $fname, $linenum, $parts[1], strlen($rom_contents), $address);
                failOperation($dirname);
            }
            // copy contents into memory map
            $start_addr = ($index * 65536) + $address;
            // now copy the data
            for ($b = 0; $b < strlen($rom_contents); $b++)
            {
                $memorymaps[$start_addr + $b] = $rom_contents[$b];
            }
        }
        else
        {
            printf("error in %s line %d: could not find %s<br>\n", $fname, $linenum, $rom_fname);
            failOperation($dirname);
        }
    }

    return $memorymaps;
}

function get_region_type($region_str) {
    $READWRITE =    "0b01010";
    $WRITETHROUGH = "0b00111";
    $READONLY =     "0b01001";
    $PASSTHROUGH =  "0b00101";
    $VRAM =         "0b10111";
    $INACTIVE =     "0b00000";

    if (strpos($region_str, "readwrite")) {
        return $READWRITE;
    } else if (strpos($region_str, "writethrough")) {
        return $WRITETHROUGH;
    } else if (strpos($region_str, "readonly")) {
        return $READONLY;
    } else if (strpos($region_str, "passthrough")) {
        return $PASSTHROUGH;
    } else if (strpos($region_str, "vram")) {
        return $VRAM;
    }

    return $INACTIVE;
}

function parse_enable_table($fname, $dirname) {
    $VRAM = "0b10111";
    $ADDR_GRANULARITY_SIZE = 256;
    $MAX_VRAM_SIZE = 2048;
    $NUMMAPS = 32;

    $granularity = $ADDR_GRANULARITY_SIZE;

    $num_address_entries = pow(2, 16) / $granularity;
    $addr_entry_bits = log($num_address_entries, 2);

    // get number of bits needed to represent the number of maps
    $config_bits = log($NUMMAPS, 2);

    // total bits needed for every entry in address enable table
    $num_entry_bits = $addr_entry_bits + $config_bits + 1;
    $num_entries = pow(2, $num_entry_bits);

    $table = str_repeat(pack("C", 0), $num_entries);

    $vram_start_addr = array();
    $vram_end_addr = array();

    for ($i = 0; $i < $NUMMAPS; $i++)
    {
        $vram_start_addr[$i] = 0;
        $vram_end_addr[$i] = 0;
    }

    $linenum = 0;
    foreach (file($fname) as $line) {
        $linenum = $linenum + 1;
        if (strlen($line) < 2) {
            continue;
        }

        // skip comments
        if ($line[0] == '#' || ($line[0] == '/' && $line[1]) == '/') {
            continue;
        }

        $parts = explode(",", $line);
        if (sizeof($parts) < 4 || sizeof($parts) > 5) {
            printf("error in %s line %d: should be 4 or 5 fields, found %d<br>\n", $fname, $linenum, sizeof($parts));
            failOperation($dirname);
        }
        
        $start_map_index = -1;
        $end_map_index = -1;
        if (strpos($parts[0], "-")) {
            sscanf($parts[0], "%d-%d", $start_map_index, $end_map_index);
        } else {
            $start_map_index = $parts[0];
            $end_map_index = $start_map_index;
        }

        if ($end_map_index < $start_map_index) {
            printf("error in %s line %d: end index %d less than start index %d<br>\n", $fname, $linenum, $end_map_index, $start_map_index);
            failOperation($dirname);
        }

        if ($end_map_index >= $NUMMAPS) {
            printf("error in %s line %d: end index %d greater than max of %d<br>\n", $fname, $linenum, $end_map_index, $NUMMAPS-1);
            failOperation($dirname);
        }

        // get address
        $addr = base_convert($parts[1], 16, 10);
        $end_addr = base_convert($parts[2], 16, 10);

        if ($end_addr < $addr) {
            printf("error in %s line %d: end address %X less than start address %X<br>\n", $fname, $linenum, $end_addr, $addr);
            failOperation($dirname);
        }

        $region_type = get_region_type($parts[3]);

        if ($region_type == $VRAM)
        {
            //printf("VRAM region<br>\n");
            // TBD, not implemented yet
        }
        else
        {
            for ($map_index = $start_map_index; $map_index <= $end_map_index; $map_index++)
            {
                for ($address = $addr; $address <= $end_addr; $address += $granularity)
                {
                    for ($rw = 0; $rw < 2; $rw++)
                    {
                        // get table address
                        // address has the following bit pattern:
                        // config(config_bits), rw, addr(addr_entry_bits)
                        $config_index = $map_index;
                        $table_addr = $config_index;
                        $table_addr <<= 1;
                        $table_addr += $rw;
                        $table_addr <<= $addr_entry_bits;

                        // get high bits of address to get the index of the address entry
                        $addr_shift = 16 - $addr_entry_bits;
                        $entry_addr = $address >> $addr_shift;
                        $table_addr += $entry_addr;

                        // get bit pattern for this entry
                        // 2 higher bits are the read (high) value
                        // 2 lower bits are the write value
                        $byteval = 0;

                        if ($rw == 1) {
                            $region_val = (int)base_convert($region_type, 2, 10);
                            $mask_val = (int)base_convert("0b01100", 2, 10);
                            $byteval = ($region_val & $mask_val) >> 2;
                        } else {
                            $region_val = (int)base_convert($region_type, 2, 10);
                            $mask_val = (int)base_convert("0b00011", 2, 10);
                            $byteval = $region_val & $mask_val;
                        }
                        $table[$table_addr] = pack("C", $byteval);
                    }
                }
            }
        }
    }

    // write table to file
    return $table;
}

if (isset($_POST["submit"])) {

    $r = rand();
    $target = sprintf("%d.zip", $r);
    $dirname = $r;
    $uploadok = 1;

    $filetype = strtolower(pathinfo(basename($_FILES["firmware_zip"]["name"]),PATHINFO_EXTENSION));
    if ($filetype != "zip") {
        echo "error, only zip files allowed.";
        $uploadok = 0;
    }

    if ($uploadok == 1) {
        if (!move_uploaded_file($_FILES["firmware_zip"]["tmp_name"], $target)) {
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

    // find enable table
    $enable_table = "";
    $s = sprintf("%s/enable_table*.csv", $dirname);
    $f = glob($s);

    if (sizeof($f) == 1) {
        $enable_table = $f[0];
    } else {
        printf("could not find enable table!\n");
        die();
    }

    // find memory set
    $s = sprintf("%s/memory_set*.csv", $dirname);
    $f = glob($s);

    if (sizeof($f) == 1) {
        $memory_set = $f[0];
    } else {
        printf("could not find memory set!\n");
        die();
    }

    // start with memory set
    $mem_set_bin = parse_memory_set($memory_set, $dirname);
    
    // create enable table
    $enable_table_bin = parse_enable_table($enable_table, $dirname);
    deleteDirectory($dirname);

    // join with bitstream and return file
    header('Content-Type: application/octet-stream');
    header('Content-Disposition: attachment; filename="romulator.bin"');
    $bitstream = file_get_contents("hardware.bin");
    // get length
    $bs_len = strlen($bitstream);
    $remainder_len = pow(2, 17) - $bs_len;
    $empty = str_repeat(pack("C", "0"), $remainder_len);

    echo($bitstream);
    echo($empty);
    echo($mem_set_bin);
    echo($enable_table_bin);
} else { 
    echo '
<html>
<body style="background-color: #000000; color: #00FF00;">
<h1>ROMulator Build</h1>
Upload a .zip file containing:<br>
Memory Set: a file named memory_set*.csv containing references to ROM files.<br>
Enable Table: a file called enable_table*.csv containing the enable table specs for your build.<br>
All ROM files referenced in the build.<br>
You will receive a file called "romulator.bin" which you can directly program onto the device.<br>
<form action="index.php" method="post" enctype="multipart/form-data">
Choose file to upload:
<input type="file" id="firmware_zip" name="firmware_zip" accept=".zip"><br>
<input type="submit" value="Upload" name="submit">
</form>
</body>
</html>
    ';
}
?>