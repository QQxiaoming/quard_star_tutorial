(:**************************************************************:)
(: Test: functx-functx-if-absent-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The first argument if it is not empty, otherwise the second argument 
 :
 : @author  W3C XML Query WG 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_if-absent.html 
 : @param   $arg the item(s) that may be absent 
 : @param   $value the item(s) to use if the item is absent 
 :) 
declare function functx:if-absent 
  ( $arg as item()* ,
    $value as item()* )  as item()* {
       
    if (exists($arg))
    then $arg
    else $value
 } ;

let $in-xml := <prices>
   <price value="29.99" discount="10.00"/>
   <price value="39.99" discount="6.00"/>
   <price value="69.99"/>
   <price value="49.99" discount=""/>
</prices>
return (data(functx:if-absent(
     $in-xml//price[1]/@discount, 0)), data(functx:if-absent(
     $in-xml//price[3]/@discount, 0)), data(functx:if-absent(
     $in-xml//price[4]/@discount, 0)))