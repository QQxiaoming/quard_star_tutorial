(:**************************************************************:)
(: Test: functx-functx-yyyyddmm-to-date-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Converts a string with format YYYYDDMM (with any delimiters) to a date 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.1 
 : @see     http://www.xqueryfunctions.com/xq/functx_yyyyddmm-to-date.html 
 : @param   $dateString the YYYYDDMM string 
 :) 
declare function functx:yyyyddmm-to-date 
  ( $dateString as xs:string? )  as xs:date? {
       
   if (empty($dateString))
   then ()
   else if (not(matches($dateString,
                        '^\D*(\d{4})\D*(\d{2})\D*(\d{2})\D*$')))
   then error(xs:QName('functx:Invalid_Date_Format'))
   else xs:date(replace($dateString,
                        '^\D*(\d{4})\D*(\d{2})\D*(\d{2})\D*$',
                        '$1-$3-$2'))
 } ;
(functx:yyyyddmm-to-date('2004/15/12'))
