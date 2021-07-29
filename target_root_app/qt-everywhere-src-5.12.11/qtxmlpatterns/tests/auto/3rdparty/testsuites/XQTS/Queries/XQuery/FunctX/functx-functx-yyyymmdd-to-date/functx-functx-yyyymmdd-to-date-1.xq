(:**************************************************************:)
(: Test: functx-functx-yyyymmdd-to-date-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Converts a string with format YYYYMMDD (with any delimiters) to a date 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.1 
 : @see     http://www.xqueryfunctions.com/xq/functx_yyyymmdd-to-date.html 
 : @param   $dateString the YYYYMMDD string 
 :) 
declare function functx:yyyymmdd-to-date 
  ( $dateString as xs:string? )  as xs:date? {
       
   if (empty($dateString))
   then ()
   else if (not(matches($dateString,
                        '^\D*(\d{4})\D*(\d{2})\D*(\d{2})\D*$')))
   then error(xs:QName('functx:Invalid_Date_Format'))
   else xs:date(replace($dateString,
                        '^\D*(\d{4})\D*(\d{2})\D*(\d{2})\D*$',
                        '$1-$2-$3'))
 } ;
(functx:yyyymmdd-to-date('2004-12-15'))
