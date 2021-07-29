(:**************************************************************:)
(: Test: functx-functx-ddmmyyyy-to-date-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Converts a string with format DDMMYYYY (with any delimiters) to a date 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.1 
 : @see     http://www.xqueryfunctions.com/xq/functx_ddmmyyyy-to-date.html 
 : @param   $dateString the DDMMYYYY string 
 :) 
declare function functx:ddmmyyyy-to-date 
  ( $dateString as xs:string? )  as xs:date? {
       
   if (empty($dateString))
   then ()
   else if (not(matches($dateString,
                        '^\D*(\d{2})\D*(\d{2})\D*(\d{4})\D*$')))
   then error(xs:QName('functx:Invalid_Date_Format'))
   else xs:date(replace($dateString,
                        '^\D*(\d{2})\D*(\d{2})\D*(\d{4})\D*$',
                        '$3-$2-$1'))
 } ;
(functx:ddmmyyyy-to-date('15/12/2004'))
