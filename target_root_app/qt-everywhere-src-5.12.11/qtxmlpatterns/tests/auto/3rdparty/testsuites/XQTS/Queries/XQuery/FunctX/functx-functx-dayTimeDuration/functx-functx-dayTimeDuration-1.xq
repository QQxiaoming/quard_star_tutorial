(:**************************************************************:)
(: Test: functx-functx-dayTimeDuration-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Construct a dayTimeDuration from a number of days, hours, etc. 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_daytimeduration.html 
 : @param   $days the number of days 
 : @param   $hours the number of hours 
 : @param   $minutes the number of minutes 
 : @param   $seconds the number of seconds 
 :) 
declare function functx:dayTimeDuration 
  ( $days as xs:decimal? ,
    $hours as xs:decimal? ,
    $minutes as xs:decimal? ,
    $seconds as xs:decimal? )  as xs:dayTimeDuration {
       
    (xs:dayTimeDuration('P1D') * functx:if-empty($days,0)) +
    (xs:dayTimeDuration('PT1H') * functx:if-empty($hours,0)) +
    (xs:dayTimeDuration('PT1M') * functx:if-empty($minutes,0)) +
    (xs:dayTimeDuration('PT1S') * functx:if-empty($seconds,0))
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
(functx:dayTimeDuration(1,6,0,0))
