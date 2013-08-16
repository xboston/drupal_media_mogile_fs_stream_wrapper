<?php

/**
 * A PHP Stream Wrapper for MogileFS
 * by Florian Anderiasch <fa at art-core dot org>
 * inspired by media_amazon and hash_wrapper
 * fixed by boston, специально для ihc.ru
 *
 * ver. 0.2.0 w
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
        /**
         * 'hosts'   => '127.0.0.1' ,
         * 'domain'  => 'test.mogile' ,
         * 'class'   => 'class2' ,
         * 'port'    => 7001 ,
         * 'output'  => 'http://127.0.0.1/mogile' ,
         */
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
        $parts      = explode('/' , $uri);
        $base       = explode('\.' , array_pop($parts));
        $this->_key = '/' . $base[0] . "";
    }

    public function getUri()
    {
        return $this->_uri;
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

    public static function getMimeType($uri , $mapping = null)
    {
        return false;
    }

    public function chmod($mode)
    {
        return true;
    }

    public function realpath()
    {
        return sprintf('ihcmogile://%s' , $this->getLocalPath());
    }

    public function dirname($uri = null)
    {

        list($stream , $target) = explode('://' , $uri , 2);

        $dirname = dirname($target);

        if ( $dirname == '.' ) {

            $dirname = '';
        }

        return $r = sprintf('%s://%s' , $stream , $dirname);
    }

    public function stream_open($uri , $mode , $options , &$opened_url)
    {

        $this->setUri($uri);
        if ( $this->_getMogileClient() ) {

            $this->_pos = 0;

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
        return lock_aquire('media_ihcmogile:' . $this->_uri);
    }

    private $position;

    public function stream_read($count)
    {

        if ( $this->_data === null && $this->_getMogileClient() ) {

            $o           = $this->_getObject();
            $this->_data = file_get_contents($o['path1']);
        }

        $p   = $this->position;
        $ret = substr($this->_data , $p , $count);
        $this->position += strlen($ret);

        return $ret;
    }

    public function stream_write($data)
    {

        if ( $this->_getMogileClient() && strlen($data) > 0 ) {

            $this->_data .= $data;

            return strlen($data);
        }

        return false;
    }

    public function stream_eof()
    {

        if ( $this->_getMogileClient() && $tmp = $this->_data ) {
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

                $stream = new self;

                $this->_mog = new MogileFs();
                $this->_mog->connect($stream->_m['hosts'] , $stream->_m['port'] , $stream->_m['domain']);

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
                $this->_object = $this->_mog->get($this->_key);
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
            $uri = $this->_uri;
        }

        $this->_getMogileClient();

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

        if ( $this->_is_dir($uri) ) {

            $stat['mode'] = 0040000; // S_IFDIR indicating directory
            $stat['mode'] |= 0777;
            $stat['2'] = $stat['mode'];

            return $stat;

        } else {

            $path = basename($uri);

            try {

                $r = $this->_mog->fileInfo('/' . $path);
            } catch ( Exception $e ) {

                return false;
            }

            $stat['size']  = $r['length'];
            $stat['mtime'] = time();

            return $stat;
        }

    }

    public function _is_dir($uri = null)
    {
        if ( $uri == null ) {
            $uri = $this->uri;
        }

        return strpos('.' , $uri);
    }
}
