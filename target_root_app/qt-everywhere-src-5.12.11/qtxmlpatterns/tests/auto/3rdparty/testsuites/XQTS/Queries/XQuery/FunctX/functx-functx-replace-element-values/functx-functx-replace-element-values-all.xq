(:**************************************************************:)
(: Test: functx-functx-replace-element-values-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Updates the content of one or more elements 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_replace-element-values.html 
 : @param   $elements the elements whose content you wish to replace 
 : @param   $values the replacement values 
 :) 
declare function functx:replace-element-values 
  ( $elements as element()* ,
    $values as xs:anyAtomicType* )  as element()* {
       
   for $element at $seq in $elements
   return element { node-name($element)}
             { $element/@*,
               $values[$seq] }
 } ;

let $in-xml := <in-xml>
   <price num="1">12</price>
   <price num="2">20</price>
   <price num="3">5</price>
</in-xml>
return (functx:replace-element-values(
   $in-xml/price,
   for $p in $in-xml/price
   return $p * 2), for $p in $in-xml/price
return functx:replace-element-values(
         $p,concat($p,'.0')))