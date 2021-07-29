(:**************************************************************:)
(: Test: functx-functx-previous-day-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The previous day 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_previous-day.html 
 : @param   $date the date 
 :) 
declare function functx:previous-day 
  ( $date as xs:anyAtomicType? )  as xs:date? {
       
   xs:date($date) - xs:dayTimeDuration('P1D')
 } ;
(functx:previous-day(xs:date('2004-01-23')))
