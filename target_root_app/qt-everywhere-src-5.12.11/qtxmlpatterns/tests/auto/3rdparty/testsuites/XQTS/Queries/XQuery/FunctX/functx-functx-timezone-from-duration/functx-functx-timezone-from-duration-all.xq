(:**************************************************************:)
(: Test: functx-functx-timezone-from-duration-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Converts an xs:dayTimeDuration into a timezone like "-05:00" or "Z" 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_timezone-from-duration.html 
 : @param   $duration the duration 
 :) 
declare function functx:timezone-from-duration 
  ( $duration as xs:dayTimeDuration )  as xs:string {
       
   if (string($duration) = ('PT0S','-PT0S'))
   then 'Z'
   else if (matches(string($duration),'-PT[1-9]H'))
   then replace(string($duration),'PT([1-9])H','0$1:00')
   else if (matches(string($duration),'PT[1-9]H'))
   then replace(string($duration),'PT([1-9])H','+0$1:00')
   else if (matches(string($duration),'-PT1[0-4]H'))
   then replace(string($duration),'PT(1[0-4])H','$1:00')
   else if (matches(string($duration),'PT1[0-4]H'))
   then replace(string($duration),'PT(1[0-4])H','+$1:00')
   else error(xs:QName('functx:Invalid_Duration_Value'))
 } ;
(functx:timezone-from-duration(
     xs:dayTimeDuration('PT0S')), functx:timezone-from-duration(
     xs:dayTimeDuration('-PT5H')), functx:timezone-from-duration(
     xs:dayTimeDuration('PT9H')))