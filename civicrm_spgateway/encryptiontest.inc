<?php
function _mcrypt_add_padding($string, $blocksize = 32) {
  $len = strlen($string);
  $pad = $blocksize - ($len % $blocksize);
  $string .= str_repeat(chr($pad), $pad);
  return $string;
}

function _mcrypt_strip_padding($string) {
  $slast = ord(substr($string, -1));
  $slastc = chr($slast);
  if (preg_match("/$slastc{" . $slast . "}/", $string)) {
    $string = substr($string, 0, strlen($string) - $slast);
    return $string;
  }
  return FALSE;
}

$key = 'MlkGHMr1Hw6u1Q1ASej33LJ64anKPnjB';
$iv = 'JEhqrJs8gzChpESi';
$str = '1234567890abcdefghijklmnopqrstuzwxyzABCDEFG';

echo "================BEGIN MCRYPT==================\n";
var_dump($str);
$paddedstr = _mcrypt_add_padding($str);
var_dump($str);
$crypt = mcrypt_encrypt(MCRYPT_RIJNDAEL_128, $key, $paddedstr, MCRYPT_MODE_CBC, $iv);
var_dump($crypt);
$hexed = bin2hex($crypt);
var_dump($hexed);
$crypt = hex2bin($hexed);
var_dump($crypt);
$decrypt = mcrypt_decrypt(MCRYPT_RIJNDAEL_128, $key, $crypt, MCRYPT_MODE_CBC, $iv);
$decrypt = _mcrypt_strip_padding($decrypt);
var_dump($decrypt);
echo "\n\n";

echo "======================RESULT====================\n";
if ($decrypt === $str) {
  echo "### Decrypt Successful!\n";
}
echo "\n\n";

echo "================BEGIN OPENSSL==================\n";
// _mcrypt_add_padding blocksize = 16 === AES-128-CBC
// _mcrypt_add_padding blocksize = 32 === AES-256-CBC
$ocrypt = openssl_encrypt($paddedstr, 'AES-256-CBC', $key, OPENSSL_RAW_DATA | OPENSSL_ZERO_PADDING, $iv);
var_dump($ocrypt);
$ohexed = bin2hex($ocrypt);
var_dump($ohexed);
if ($ohexed === $hexed) {
  echo "### Openssl encrypt succeed! ####\n";
}
$ocrypt = hex2bin($ohexed);
var_dump($ocrypt);
$odecrypt =  openssl_decrypt($ocrypt, 'AES-256-CBC', $key, OPENSSL_RAW_DATA | OPENSSL_ZERO_PADDING, $iv);
$odecrypt = _mcrypt_strip_padding($odecrypt);
var_dump($odecrypt);
echo "\n\n";
echo "======================RESULT====================\n";
if ($odecrypt === $str) {
  echo "### Decrypt Successful!\n";
}
echo "\n\n";

echo "===============BEGIN SPGATEWAY TEST================\n";
if (!function_exists('_civicrm_spgateway_encrypt')) {
  echo "Warning: You should use 'drush scr encryptiontest.inc' to run test below\n";
  echo "Exit program\n";
  exit();
}
$mcrypt = _civicrm_spgateway_encrypt($key, $iv, $str, 'mcrypt');
$ocrypt = _civicrm_spgateway_encrypt($key, $iv, $str, 'openssl');
echo $mcrypt."\n";
echo $ocrypt."\n";
if ($ocrypt === $mcrypt) {
  echo "### BOTH encryption have same encrypt! ####\n";
}
else {
  echo "ERROR: something wrong when encrypt.";
}


$mdecrypted = _civicrm_spgateway_decrypt($key, $iv, $mcrypt, 'mcrypt');
$odecrypted = _civicrm_spgateway_decrypt($key, $iv, $ocrypt, 'openssl');
if ($mdecrypted === $odecrypted) {
  echo "### BOTH decryption have same result! ####\n";
}
else {
  echo "ERROR: something wrong when decrypt.";
}

if ($mdecrypted === $str) {
  echo "# mcrypt descrypt success ! \n";
}
if ($odecrypted === $str) {
  echo "# openssl descrypt success ! \n";
}