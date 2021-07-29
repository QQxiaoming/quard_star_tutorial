(:**************************************************************:)
(: Test: functx-functx-month-abbrev-en-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The month of a date as an abbreviated word (Jan, Feb, etc.) 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_month-abbrev-en.html 
 : @param   $date the date 
 :) 
declare function functx:month-abbrev-en 
  ( $date as xs:anyAtomicType? )  as xs:string? {
       
   ('Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
    'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec')
    [month-from-date(xs:date($date))]
 } ;
(functx:month-abbrev-en(
     xs:dateTime('2004-01-23T12:00:13')))
