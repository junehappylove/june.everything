<?php
$file_name = "ajax.jpg";
$pic_data=$_POST["picData"];
$real_picture_data=base64_decode($pic_data);

$file=fopen($file_name,"w");
fwrite($file,$real_picture_data);
fclose($file);
print "{'savestatus':'ok'}";
?> 

