(:**************************************************************:)
(: Test: functx-functx-wrap-values-in-elements-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
declare namespace new = "http://newns"; 
(:~
 : Wraps a sequence of atomic values in XML elements 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_wrap-values-in-elements.html 
 : @param   $values the values to wrap in elements 
 : @param   $elementName the name of the elements to construct 
 :) 
declare function functx:wrap-values-in-elements 
  ( $values as xs:anyAtomicType* ,
    $elementName as xs:QName )  as element()* {
       
   for $value in $values
   return element {$elementName} {$value}
 } ;
(functx:wrap-values-in-elements(
   (1,2,3),
   xs:QName('num')), functx:wrap-values-in-elements(
      (1,2,3),
      xs:QName('new:num')), functx:wrap-values-in-elements(
   (1,2,3),
   QName('http://newns','num')), functx:wrap-values-in-elements(
   (1,2,3),
   QName('http://newns','new:num')))