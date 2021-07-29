(:**************************************************************:)
(: Test: functx-functx-sort-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Sorts a sequence of values or nodes 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_sort.html 
 : @param   $seq the sequence to sort 
 :) 
declare function functx:sort 
  ( $seq as item()* )  as item()* {
       
   for $item in $seq
   order by $item
   return $item
 } ;

let $in-xml := <in-xml>
  <f>c</f>
  <f>a</f>
  <e>b</e>
</in-xml>
return (functx:sort($in-xml/*))
