(:**************************************************************:)
(: Test: functx-functx-is-leap-year-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether a date falls in a leap year 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-leap-year.html 
 : @param   $date the date or year 
 :) 
declare function functx:is-leap-year 
  ( $date as xs:anyAtomicType? )  as xs:boolean {
       
    for $year in xs:integer(substring(string($date),1,4))
    return ($year mod 4 = 0 and
            $year mod 100 != 0) or
            $year mod 400 = 0
 } ;
(functx:is-leap-year(xs:date('2004-01-23')))
