(:**************************************************************:)
(: Test: functx-functx-days-in-month-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Number of days in the month 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_days-in-month.html 
 : @param   $date the date 
 :) 
declare function functx:days-in-month 
  ( $date as xs:anyAtomicType? )  as xs:integer? {
       
   if (month-from-date(xs:date($date)) = 2 and
       functx:is-leap-year($date))
   then 29
   else
   (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)
    [month-from-date(xs:date($date))]
 } ;

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
(functx:days-in-month('2005-02-15'))
