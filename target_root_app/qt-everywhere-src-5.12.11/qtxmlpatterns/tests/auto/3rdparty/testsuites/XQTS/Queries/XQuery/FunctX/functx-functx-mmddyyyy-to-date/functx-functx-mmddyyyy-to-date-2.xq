(:**************************************************************:)
(: Test: functx-functx-mmddyyyy-to-date-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Converts a string with format MMDDYYYY (with any delimiters) to a date 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_mmddyyyy-to-date.html 
 : @param   $dateString the MMDDYYYY string 
 :) 
declare function functx:mmddyyyy-to-date 
  ( $dateString as xs:string? )  as xs:date? {
       
   if (empty($dateString))
   then ()
   else if (not(matches($dateString,
                        '^\D*(\d{2})\D*(\d{2})\D*(\d{4})\D*$')))
   then error(xs:QName('functx:Invalid_Date_Format'))
   else xs:date(replace($dateString,
                        '^\D*(\d{2})\D*(\d{2})\D*(\d{4})\D*$',
                        '$3-$1-$2'))
 } ;
(functx:mmddyyyy-to-date('12152004'))
