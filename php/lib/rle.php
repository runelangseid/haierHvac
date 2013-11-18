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

        $numbers = array( '1' => chr(32),   // SPACE ascii 32, increase by 8      220-359
                          '2' => chr(40),   // (                                  360-439
                          '3' => chr(48),   // 0                                  440-519
                          '4' => chr(56),   // 8                                  520-599
                          '5' => chr(64),   // @                                  600-679
                          '6' => chr(72),   // H                                  680-759
                          '7' => chr(80),   // P                                  760-839
                          '8' => chr(88),   // X                                  840-919
                          '9' => chr(97),   // a   - special, increase of 9       920-999
                          '0' => chr(104),  // h                                 1000-1079
                    );

        $k =       array( 'A' => chr(112),  // p                                  1080-1159
                          'B' => chr(120),  // x                                  1160-1239
                          'C' => chr(126),  // ~                                  1240-1319
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