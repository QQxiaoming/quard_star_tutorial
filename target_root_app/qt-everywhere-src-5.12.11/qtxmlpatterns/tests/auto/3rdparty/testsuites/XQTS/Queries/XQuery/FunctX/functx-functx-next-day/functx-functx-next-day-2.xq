(:**************************************************************:)
(: Test: functx-functx-next-day-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The next day 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_next-day.html 
 : @param   $date the date 
 :) 
declare function functx:next-day 
  ( $date as xs:anyAtomicType? )  as xs:date? {
       
   xs:date($date) + xs:dayTimeDuration('P1D')
 } ;
(functx:next-day(
     xs:dateTime('2005-12-31T12:00:13')))
