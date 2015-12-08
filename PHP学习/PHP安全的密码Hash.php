<?php 
/**
 * 使用 hash_pbkdf2() php要大于5.5
 */

<?php
/*
 * Password Hashing With PBKDF2
 * STRAYA!
 */

class hash{

	const PBKDF2_HASH_ALGORITHM =  'sha256';
	const PBKDF2_ITERATIONS     =  1024;
	const PBKDF2_SALT_BYTE_SIZE =  24;
	const PBKDF2_HASH_BYTE_SIZE =  24;

    /**
     *
     * Settor
     *
     * @access    public
     * @param    string    $name
     * @param    mixed    $value
     *
     */
    public function __set( $name, $value )
    {
        switch( $name )
        {
            case 'salt':
            case 'hash':
            case 'password':
            	$this->$name = $value;
	            break;

            default: throw new Exception( "Unable to set $name" );
        }
    }

    /**
     *
     * Gettor
     *
     * @access    public
     * @param    string    $name
     *
     */
    public function __get( $name )
    {
        switch( $name )
        {
            case 'salt':
            case 'hash':
            case 'password':
	            return $this->$name;
            default: throw new Exception( "Unable to get $name" );
        }
    }

    /**
     *
     * Create the hash
     *
     * @access    public
     *
     */
    public function createHash()
    {
        $this->salt = base64_encode(mcrypt_create_iv( self::PBKDF2_SALT_BYTE_SIZE, MCRYPT_DEV_URANDOM ) );
        $this->hash = base64_encode( $this->pbkdf2( $this->password, $this->salt, self::PBKDF2_HASH_BYTE_SIZE, true ) );
    }

    /**
    *
    * Validate password
    *
    * @access    public
    * @param    string    $password
    * @param    string    $correct_hash
    * @return    bool
    *
    */
    public function isValid()
    {
        $pbkdf2 = base64_decode( $this->hash );
        return $this->compare( $pbkdf2, $this->pbkdf2( $this->password, $this->salt, strlen($pbkdf2),true ) );
    }

    /**
    *
    * Compares two strings in length-constant time.
    *
    * @access    private
    * @param    string    $a
    * @param    string    $b
    * @return    bool
    *
    */
    private function compare($a, $b)
    {
        $diff = strlen($a) ^ strlen($b);
        for( $i = 0; $i < strlen($a) && $i < strlen($b); $i++ )
        {
            $diff |= ord( $a[$i]) ^ ord($b[$i] );
        }
        return $diff === 0;
    }

    /**
    *
    *  PBKDF2 key derivation
    * @access    public
    * @param    string    $password    Password string
    * @param    string    $salt        A salt that is unique to the password
    * @param    int    $key_len    The length of the derived key in bytes
    * @param    bool    $raw_data    If true, the key is returned in raw binary format. Hex encoded otherwise
    * @return    string    A $key_len-byte key derived from the password and salt
    * @see        https://www.ietf.org/rfc/rfc2898.txt
    * @see        https://www.ietf.org/rfc/rfc6070.txt
    *
    */
    public function pbkdf2( $password, $salt, $key_len, $raw_data = false )
    {
        $algorithm = strtolower( self::PBKDF2_HASH_ALGORITHM ); // SHA256

        if ( function_exists("hash_pbkdf2") )
        {
            // The output length is in NIBBLES (4-bits) if $raw_data is false!
            if ( !$raw_data ) {
                $key_len = $key_len * 2;
            }
            return hash_pbkdf2( $algorithm, $password, $salt, self::PBKDF2_ITERATIONS, $key_len, $raw_data);
        }

        $hash_length = strlen( hash( $algorithm, "", true ) );
        $block_count = ceil( $key_len / $hash_length );

        $output = "";
        for( $i = 1; $i <= $block_count; $i++ )
        {
            // $i encoded as 4 bytes, big endian.
            $last = $salt . pack( "N", $i );
            // first iteration
            $last = $xorsum = hash_hmac( $algorithm, $last, $password, true );
            // perform the other $count - 1 iterations
            for ( $j = 1; $j < $count; $j++ )
            {
                $xorsum ^= ( $last = hash_hmac( $algorithm, $last, $password, true ) );
            }
            $output .= $xorsum;
        }

        if($raw_data)
        {
            return substr( $output, 0, $key_len );
        }
        return bin2hex( substr( $output, 0, $key_len ) );
    }

} // end of class


#test

$hash = new hash;
$hash->password = "password123";
$hash->createHash();
echo $hash->salt;
echo $hash->hash;

#validate a password
$hash->password = 'password123';
$hash->salt = "cBePBbLWqYzqoxnigbX+uTGHY98AEhMR";
$hash->hash = " A392DhczIMXkUk0u8cFy2cUSPUgMTdK7";

if ($hash->isValid())
{

}
?>
