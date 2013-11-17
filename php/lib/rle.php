<?php

class Rle
{
    public static function encode( $raw, $asString = true )
    {
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

        $numbers = array( '1' => chr(32),   // SPACE ascii 80, increase by 8
                          '2' => chr(40),   // (
                          '3' => chr(48),   // 0
                          '4' => chr(56),   // 8
                          '5' => chr(64),   // @
                          '6' => chr(72),   // H
                          '7' => chr(80),   // P
                          '8' => chr(88),   // X
                          '9' => chr(97),   // a   - special, increase of 9
                          '0' => chr(104),  // h
                    );

    $k = array( 'A' => chr(112),
                'B' => chr(120),
                'C' => chr(127),
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
}