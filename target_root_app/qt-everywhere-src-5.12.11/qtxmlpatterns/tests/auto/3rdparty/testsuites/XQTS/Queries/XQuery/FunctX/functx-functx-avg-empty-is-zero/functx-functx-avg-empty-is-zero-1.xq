(:**************************************************************:)
(: Test: functx-functx-avg-empty-is-zero-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The average, counting "empty" values as zero 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_avg-empty-is-zero.html 
 : @param   $values the values to be averaged 
 : @param   $allNodes the sequence of all nodes to find the average over 
 :) 
declare function functx:avg-empty-is-zero 
  ( $values as xs:anyAtomicType* ,
    $allNodes as node()* )  as xs:double {
       
   if (empty($allNodes))
   then 0
   else sum($values[string(.) != '']) div count($allNodes)
 } ;

let $in-xml := <prices>
   <price value="29.99" discount="10.00"/>
   <price value="39.99" discount="6.00"/>
   <price value="69.99"/>
   <price value="49.99" discount=""/>
</prices>
return (functx:avg-empty-is-zero(
   $in-xml//price/@discount, $in-xml//price))
