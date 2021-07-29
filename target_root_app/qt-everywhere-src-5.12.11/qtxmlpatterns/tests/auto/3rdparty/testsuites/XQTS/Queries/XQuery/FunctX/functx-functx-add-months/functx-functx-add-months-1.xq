(:**************************************************************:)
(: Test: functx-functx-add-months-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Adds months to a date 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_add-months.html 
 : @param   $date the date 
 : @param   $months the number of months to add 
 :) 
declare function functx:add-months 
  ( $date as xs:anyAtomicType? ,
    $months as xs:integer )  as xs:date? {
       
   xs:date($date) + functx:yearMonthDuration(0,$months)
 } ;

(:~
 : The first argument if it is not blank, otherwise the second argument 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_if-empty.html 
 : @param   $arg the node that may be empty 
 : @param   $value the item(s) to use if the node is empty 
 :) 
declare function functx:if-empty 
  ( $arg as item()? ,
    $value as item()* )  as item()* {
       
  if (string($arg) != '')
  then data($arg)
  else $value
 } ;

(:~
 : Construct a yearMonthDuration from a number of years and months 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_yearmonthduration.html 
 : @param   $years the number of years 
 : @param   $months the number of months 
 :) 
declare function functx:yearMonthDuration 
  ( $years as xs:decimal? ,
    $months as xs:integer? )  as xs:yearMonthDuration {
       
    (xs:yearMonthDuration('P1M') * functx:if-empty($months,0)) +
    (xs:yearMonthDuration('P1Y') * functx:if-empty($years,0))
 } ;
(functx:add-months(xs:date('2004-01-23'),1))
