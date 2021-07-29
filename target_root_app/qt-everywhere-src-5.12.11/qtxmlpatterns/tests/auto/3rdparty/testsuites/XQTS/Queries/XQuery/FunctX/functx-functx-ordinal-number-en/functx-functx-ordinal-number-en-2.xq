(:**************************************************************:)
(: Test: functx-functx-ordinal-number-en-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Reformats a number as an ordinal number, e.g. 1st, 2nd, 3rd. 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_ordinal-number-en.html 
 : @param   $num the number 
 :) 
declare function functx:ordinal-number-en 
  ( $num as xs:integer? )  as xs:string {
       
   concat(xs:string($num),
         if (matches(xs:string($num),'[04-9]$|1[1-3]$')) then 'th'
         else if (ends-with(xs:string($num),'1')) then 'st'
         else if (ends-with(xs:string($num),'2')) then 'nd'
         else if (ends-with(xs:string($num),'3')) then 'rd'
         else '')
 } ;
(functx:ordinal-number-en(12))
