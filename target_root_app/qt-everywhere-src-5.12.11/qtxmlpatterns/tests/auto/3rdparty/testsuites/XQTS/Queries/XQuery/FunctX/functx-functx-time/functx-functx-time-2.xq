(:**************************************************************:)
(: Test: functx-functx-time-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Pads an integer to a desired length by adding leading zeros 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_pad-integer-to-length.html 
 : @param   $integerToPad the integer to pad 
 : @param   $length the desired length 
 :) 
declare function functx:pad-integer-to-length 
  ( $integerToPad as xs:anyAtomicType? ,
    $length as xs:integer )  as xs:string {
       
   if ($length < string-length(string($integerToPad)))
   then error(xs:QName('functx:Integer_Longer_Than_Length'))
   else concat
         (functx:repeat-string(
            '0',$length - string-length(string($integerToPad))),
          string($integerToPad))
 } ;

(:~
 : Repeats a string a given number of times 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_repeat-string.html 
 : @param   $stringToRepeat the string to repeat 
 : @param   $count the desired number of copies 
 :) 
declare function functx:repeat-string 
  ( $stringToRepeat as xs:string? ,
    $count as xs:integer )  as xs:string {
       
   string-join((for $i in 1 to $count return $stringToRepeat),
                        '')
 } ;

(:~
 : Construct a time from an hour, minute and second 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_time.html 
 : @param   $hour the hour 
 : @param   $minute the minute 
 : @param   $second the second 
 :) 
declare function functx:time 
  ( $hour as xs:anyAtomicType ,
    $minute as xs:anyAtomicType ,
    $second as xs:anyAtomicType )  as xs:time {
       
   xs:time(
     concat(
       functx:pad-integer-to-length(xs:integer($hour),2),':',
       functx:pad-integer-to-length(xs:integer($minute),2),':',
       functx:pad-integer-to-length(xs:integer($second),2)))
 } ;
(functx:time('20','6','12'))
