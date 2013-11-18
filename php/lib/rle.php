<?php

class Rle
{
    public static function encode( $raw, $asString = true )
    {
        $raw = self::cleanup( $raw );

        $rawEncoded = array();

        $u = array_unique( $raw );
        $k = array( 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q' );
        $keys = array();

        // Assign keys to values
        $ii = 0;
        foreach ( $u as $i => $e )
        {
            //$keys[$k[$v]] = $e;
            $keys[$e] = $k[$ii];
            $ii++;
        }

        // Replace values with keys
        foreach ( $raw as $i => $r )
        {
            $rawEncoded[] = $keys[$r];
        }

        //var_dump( 'u', $u );
        //var_dump( 'keys', $keys );
        //var_dump( 'raw count', count( $raw ) );
        //echo 'uniqe ' . count( $u ) . "\n";

        $new = array();

        // Run-length encoding
        $last = 0;
        $buffer = array();
        $rawRunEncoded = array();
        foreach ( $rawEncoded as $i => $r )
        {
            // if new value - append buffer to data
            if ( $last != $r )
            {
                if ( count( $buffer ) > 1 )
                {
                    $rawRunEncoded[] = count( $buffer );
                }
                $rawRunEncoded[] = $last;
                $buffer = array();
            }

            $buffer[] = $r;
            $last = $r;
        }

        if ( !$asString )
        {
            return $rawRunEncoded;
        }

        $s = "";
        foreach( $rawRunEncoded as $r )
        {
            $s .= $r;
        }
        return $s;
    }

    /* Convert rle to someting we can send via the tellstick protocol (raw)
     * @param String rle converted string
     * @return Converted string
     */
    public static function convert( $str )
    {
        $r = array();

        $numbers = array( '1' => '\x0A',   // 
                          '2' => '\x14',   // 
                          '3' => '\x1E',   // 
                          '4' => '\x28',   // 
                          '5' => '\x32',   // 
                          '6' => '\x3C',   // 
                          '7' => '\x46',   // 
                          '8' => '\x50',   // 
                          '9' => '\x5A',   // 
                          '0' => '\x64',  // 
                    );

        $k =       array( 'A' => '\x6E',  // 
                          'B' => '\x78',  // 
                          'C' => '\x7F',  // 
                        );


        $chars = str_split($str);
        foreach( $chars as $c )
        {
            if ( array_key_exists( $c, $k ) )
            {
                $r[] = $k[$c];
            }
            elseif ( array_key_exists( $c, $numbers ) )
            {
                $r[] = $numbers[$c];
            }

        }

        return implode( "", $r );
    }

    public static function cleanup( $data )
    {
        $r = array();
        foreach ( $data as $a )
        {
            if ( $a > 1000 )
            {
                $r[] = 550;
            }
            else
            {
                $r[] = 1600;
            }
        }

        return $r;
    }

}