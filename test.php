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


$client = new MogileFs();
$client->connect('46.254.22.222' , 7001 , 'test.mogile');
//$paths = $client->fileInfo('/lkLK69KRpwE.jpg');
$client->put('5555.jpg', '/girl2.jpg', 'class2');
$paths = $client->get('/girl2.jpg');
$client->close();

print_r($paths);
die;





require './MediaIhcMogileStreamWrapper.inc';

stream_wrapper_register('ihcmogile' , 'MediaIhcMogileStreamWrapper');

$f = 'ihcmogile://lkLK69KRpwE.jpg';
echo file_exists($f);

die;

$fileKey = '/video/TWRFfa_T_b_NI.jpg';

$x = 'test5';

//file_put_contents(sprintf('ihcmogilefs://%s' , $fileKey) , $x);
file_get_contents(sprintf('ihcmogilefs://%s' , $fileKey) , $x);

die;

$client = new MogileFs();
$client->connect('46.254.22.222' , 7001 , 'files.com');
//$r = $client->put($x , $fileKey , 'files' , false);

//filesize('ihcmogile://video/TWRFfa_T_b_NI.jpg');
filesize('ihcmogilefs:///TWRFfa_T_b_NI.jpg');


echo is_dir(sprintf('ihcmogilefs://%s' , 'video')) ? 'да' : 'нет';
die;
echo '111' . PHP_EOL;
echo file_exists(sprintf('ihcmogilefs://%s' , $fileKey)) ? 'да' : 'нет';
echo PHP_EOL;
echo '222' . PHP_EOL;

