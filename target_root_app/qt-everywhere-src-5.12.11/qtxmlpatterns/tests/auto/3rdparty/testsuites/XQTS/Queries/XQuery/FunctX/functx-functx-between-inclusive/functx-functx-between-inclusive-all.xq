(:**************************************************************:)
(: Test: functx-functx-between-inclusive-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether a value is between two provided values, or equal to one of them 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_between-inclusive.html 
 : @param   $value the value to be tested 
 : @param   $minValue the minimum value 
 : @param   $maxValue the maximum value 
 :) 
declare function functx:between-inclusive 
  ( $value as xs:anyAtomicType? ,
    $minValue as xs:anyAtomicType ,
    $maxValue as xs:anyAtomicType )  as xs:boolean {
       
   $value >= $minValue and $value <= $maxValue
 } ;
(functx:between-inclusive(55, 1, 1000), functx:between-inclusive(1, 1, 1000), functx:between-inclusive(1200, 1, 1000), functx:between-inclusive('b', 'b', 'd'), functx:between-inclusive(xs:date('2004-10-31'),
                      xs:date('2004-10-15'),
                      xs:date('2004-11-01')))