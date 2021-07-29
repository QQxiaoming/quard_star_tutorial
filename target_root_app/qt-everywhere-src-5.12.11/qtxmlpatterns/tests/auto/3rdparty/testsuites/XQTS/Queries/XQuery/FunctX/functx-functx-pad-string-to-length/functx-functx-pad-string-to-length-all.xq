(:**************************************************************:)
(: Test: functx-functx-pad-string-to-length-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Pads a string to a desired length 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_pad-string-to-length.html 
 : @param   $stringToPad the string to pad 
 : @param   $padChar the character(s) to use as padding 
 : @param   $length the desired length 
 :) 
declare function functx:pad-string-to-length 
  ( $stringToPad as xs:string? ,
    $padChar as xs:string ,
    $length as xs:integer )  as xs:string {
       
   substring(
     string-join (
       ($stringToPad, for $i in (1 to $length) return $padChar)
       ,'')
    ,1,$length)
 } ;
(functx:pad-string-to-length('abc', '*', 6), functx:pad-string-to-length('abcdef', '*', 4), functx:pad-string-to-length('', '*', 4))