<?php

/**
 * A PHP Stream Wrapper for MogileFS
 *  by Florian Anderiasch <fa at art-core dot org>
 *  inspired by media_amazon and hash_wrapper
 *
 */

class MediaIhcMogileStreamWrapper implements DrupalStreamWrapperInterface
{
    protected $_uri;

    protected $_mog;

    protected $_pos;

    protected $_key;

    protected $_object;

    protected $_data;

    protected $_m = array(
        'options' => array(
            'socketTimeout'        => 5 ,
            'streamTimeoutSeconds' => 5 ,
        ) ,
        'hosts'   => '127.0.0.1' ,
        'domain'  => 'files.com' ,
        'class'   => 'media' ,
        'port'    => '6001' ,
        'output'  => 'http://dl.example.com/' ,

    );

    public function __construct()
    {

        if ( !isset($this->_m['hosts']) ) {
            $this->_m['hosts'] = variable_get('media_ihcmogile_hosts' , '');
        }
        if ( !isset($this->_m['domain']) ) {
            $this->_m['domain'] = variable_get('media_ihcmogile_domain' , '');
        }
        if ( !isset($this->_m['class']) ) {
            $this->_m['class'] = variable_get('media_ihcmogile_class' , '');
        }
        if ( !isset($this->_m['port']) ) {
            $this->_m['port'] = variable_get('media_ihcmogile_port' , '');
        }
        if ( !isset($this->_m['output']) ) {
            $this->_m['output'] = variable_get('media_ihcmogile_output' , '');
        }
    }

    public function setUri($uri)
    {
        $this->_uri = $uri;
        //error_log("uri is $uri" . PHP_EOL);
        $parts      = explode('/' , $uri);
        $base       = explode('\.' , array_pop($parts));
        $this->_key = '/'.$base[0] . "";
        //error_log("key is $this->_key " . PHP_EOL);

    }

    public function getUri()
    {
        return $this->_uri;
    }

    protected function getTarget($uri = null)
    {

    }

    protected function getLocalPath($uri = null)
    {
        if ( !isset($uri) ) {
            $uri = $this->uri;
        }

        $path = str_replace('ihcmogilefs:/' , '' , $uri);
        $path = str_replace('ihcmogile:/' , '' , $path);

        $path = trim($path , '/');

        return $path;
    }

    public function getExternalUrl()
    {
        return sprintf('%s%s' , $this->_m['output'] , $this->_key);
    }

    //@TODO
    public static function getMimeType($uri , $mapping = null)
    {
        if ( !isset($mapping) ) {
            $mapping = 0;
        }

        return false;
    }

    public function chmod($mode)
    {
        return true;
    }

    //@TODO
    public function realpath()
    {
        return sprintf('ihcmogilefs://%s' , $this->getLocalPath());
    }

    public function dirname($uri = null)
    {
        return 'none';
    }

    public function stream_open($uri , $mode , $options , &$opened_url)
    {
        $this->setUri($uri);
        if ( $this->_getMogileClient() ) {
            $this->_pos = 0;
            error_log("opened $uri" . PHP_EOL);

            return true;
        }
    }

    public function stream_close()
    {

        $k = $this->_key;
        $this->_mog->put($this->_data , $k , $this->_m['class'] , false);

        $this->_pos    = 0;
        $this->_object = 0;

        return true;
    }

    public function stream_lock($operation)
    {
        return lock_aquire('media_mogilefs:' . $this->_uri);
    }

    public function stream_read($count)
    {
        if ( $this->_getMogileClient() ) {
            error_log("reading $this->_pos $count" . PHP_EOL);
            $data = substr($this->_getObject() , $this->_pos , $count);
            $this->_pos += $count;

            return $data;
        }

        return false;
    }

    public function stream_write($data)
    {
        //@TODO
        if ( $this->_getMogileClient() && strlen($data) > 0 ) {

            $this->_data .= $data;

            return strlen($data);
        }

        return false;
    }

    public function stream_eof()
    {
        if ( $this->_getMogileClient() && $tmp = $this->_getObject() ) {
            return $this->_pos >= strlen($tmp);
        }

        return true;
    }

    public function stream_seek($offset , $whence)
    {
        switch ( $whence ) {
            case SEEK_SET:
                if ( $this->_getMogileClient() && $tmp = $this->_getObject() ) {
                    if ( $offset < strlen($tmp) && $offset >= 0 ) {
                        $this->_pos = $offset;

                        return true;
                    }
                }
            case SEEK_CUR:
                if ( $offset >= 0 ) {
                    $this->_pos += $offset;

                    return true;
                }
            case SEEK_END:
                if ( $this->_getMogileClient() && $tmp = $this->_getObject() ) {
                    if ( strlen($tmp) + $offset >= 0 ) {
                        $this->_pos = strlen($tmp) + $offset;

                        return true;
                    }
                }
            default:
                return false;
        }
        return false;
    }

    public function stream_flush()
    {
        return true;
    }

    public function stream_tell()
    {
        return $this->_pos;
    }

    public function _stream_stat()
    {

    }

    public function unlink($uri)
    {
        if ( $uri == $this->_uri ) {
            $this->_mog->delete($this->_key);

            return true;
        }

        return false;
    }

    public function rename($from_uri , $to_uri)
    {
        return false;
        //$this->_mog->rename($this->_key, $newkey);
    }

    public function mkdir($uri , $mode , $options)
    {
        return true;
    }

    public function rmdir($uri , $options)
    {
        return false;
    }

    public function _url_stat($uri , $flags)
    {
    }

    protected function _getMogileClient()
    {
        if ( is_null($this->_mog) ) {
            try {

                $this->_mog = new MogileFs();
                $this->_mog->connect($this->_m['hosts'] , $this->_m['port'] , $this->_m['domain']);

            } catch ( RequestCore_Exception $e ) {

                drupal_set_message('There was a problem connection to MogileFS' , 'error');
            } catch ( Exception $e ) {

                drupal_set_message('There was a problem using MogileFS: ' . $e->getMessage() , 'error');
            }
        }

        return $this->_mog;
    }

    protected function _getObject()
    {
        if ( !$this->_object ) {
            try {
                $this->_object = $this->_mog->getFileData($this->_key);
            } catch ( Exception $e ) {
                return null;
            }
        }

        return $this->_object;
    }

    public function dir_opendir($uri , $options)
    {
        return true;
    }

    public function dir_readdir()
    {
        return false;
    }

    public function dir_rewinddir()
    {
        return true;
    }

    public function dir_closedir()
    {
        return true;
    }

    public function url_stat($uri , $flags)
    {
        return $this->_stat($uri);
    }

    public function stream_stat()
    {
        return $this->_stat();
    }

    public function _stat($uri = null)
    {

        if ( $uri == null ) {
            $uri = $this->uri;
        }

        $path = $this->getLocalPath($uri);

        $this->_getMogileClient();

        //echo "\n" . $path . '::' . $uri . "\n";

        try {

            $r = $this->_mog->fileInfo('/' . $path);
        } catch ( Exception $e ) {

            $r = false;
        }


        if ( true) {

            //die("есть файло");

            $mode = 0100000; // S_IFREG indicating file
            $mode |= 0777; // everything is writeable
            $stat = array(
                '0'       => 0 ,
                '1'       => 0 ,
                '2'       => $mode ,
                '3'       => 0 ,
                '4'       => 0 ,
                '5'       => 0 ,
                '6'       => 0 ,
                '7'       => 0 ,
                '8'       => 0 ,
                '9'       => 0 ,
                '10'      => 0 ,
                '11'      => 0 ,
                '12'      => 0 ,
                'dev'     => 0 ,
                'ino'     => 0 ,
                'mode'    => $mode ,
                'nlink'   => 0 ,
                'uid'     => 0 ,
                'gid'     => 0 ,
                'rdev'    => 0 ,
                'size'    => 0 ,
                'atime'   => 0 ,
                'mtime'   => 0 ,
                'ctime'   => 0 ,
                'blksize' => 0 ,
                'blocks'  => 0 ,
            );

            // check if directory
            if ( !is_array($r) ) {
                $stat['mode'] = 0040000; // S_IFDIR indicating directory
                $stat['mode'] |= 0777;
                $stat['2'] = $stat['mode'];

                //echo "каталог" . $uri."\n";

                return $stat;
            } else {

                //print_r($r);
                //echo "файл" . $uri."\n";

                $stat['size']  = $r['length'];
                $stat['mtime'] = time();

                return $stat;
            }
        }

        return false;
    }

    public function _is_dir($uri = null)
    {
        if ( $uri == null ) {
            $uri = $this->uri;
        }
        $path = $this->getLocalPath($uri);

        //die($path);

        return false;
    }


}
