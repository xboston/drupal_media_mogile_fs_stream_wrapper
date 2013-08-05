<?php

//ini_set('display_errors', 'on');
//error_reporting(E_ALL);

function variable_get($name , $def)
{
}

function drupal_set_message($t)
{
}

interface DrupalStreamWrapperInterface
{
}

;

require './MediaIhcMogileStreamWrapper.inc';

stream_wrapper_register('ihcmogilefs' , 'MediaIhcMogileStreamWrapper');

$fileKey = '/video/TWRFfa_T_b_NI.jpg1';

$x = 'test5';

//file_put_contents(sprintf('ihcmogilefs://%s' , $fileKey) , $x);

$client = new MogileFs();
$client->connect('127.0.0.1' , 6001 , 'files.com');
//$r = $client->put($x , $fileKey , 'files' , false);

//filesize('ihcmogile://video/TWRFfa_T_b_NI.jpg');
filesize('ihcmogilefs:///TWRFfa_T_b_NI.jpg');


echo is_dir(sprintf('ihcmogilefs://%s' , 'video')) ? 'да' : 'нет';
die;
echo '111' . PHP_EOL;
echo file_exists(sprintf('ihcmogilefs://%s' , $fileKey)) ? 'да' : 'нет';
echo PHP_EOL;
echo '222' . PHP_EOL;

