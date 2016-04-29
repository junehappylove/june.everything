<?php

$file_name = $_POST["phoneCode"];

$pic_data=$_POST["picData"];

//去除两端的单引号
$pic_data=mb_substr($pic_data,1);
$pic_data=mb_substr($pic_data,0,strlen($pic_data)-1);


$real_picture_data=base64_decode($pic_data);

$file=fopen($file_name,"w");
fwrite($file,$real_picture_data);
fclose($file);

print "{'savestatus':'ok'}";

?> 

