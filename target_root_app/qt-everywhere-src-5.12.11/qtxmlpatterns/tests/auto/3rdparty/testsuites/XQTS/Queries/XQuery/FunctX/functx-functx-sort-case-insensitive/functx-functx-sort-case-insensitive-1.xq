(:**************************************************************:)
(: Test: functx-functx-sort-case-insensitive-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Sorts a sequence of values or nodes regardless of capitalization 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_sort-case-insensitive.html 
 : @param   $seq the sequence to sort 
 :) 
declare function functx:sort-case-insensitive 
  ( $seq as item()* )  as item()* {
       
   for $item in $seq
   order by upper-case(string($item))
   return $item
 } ;

let $in-xml := <in-xml>
  <f>a</f>
  <f>c</f>
  <e>B</e>
</in-xml>
return (functx:sort-case-insensitive(('a','c','B')))
