<?php
$file_name = "ajax";
$pic_data=$_POST["picData"];
$pic_ext=$_POST["picExt"];
$real_picture_data=base64_decode($pic_data);

$file=fopen($file_name.$pic_ext,"w");
fwrite($file,$real_picture_data);
fclose($file);
echo "<a href='".$file_name.$pic_ext."'>click here</a>";
?> 

