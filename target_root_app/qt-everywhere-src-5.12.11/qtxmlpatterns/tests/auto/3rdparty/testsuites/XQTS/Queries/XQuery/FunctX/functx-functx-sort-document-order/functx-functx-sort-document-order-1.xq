(:**************************************************************:)
(: Test: functx-functx-sort-document-order-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Sorts a sequence of nodes in document order 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_sort-document-order.html 
 : @param   $seq the sequence to sort 
 :) 
declare function functx:sort-document-order 
  ( $seq as node()* )  as node()* {
       
   $seq/.
 } ;

let $in-xml := <in-xml>
   <a>123</a>
   <b>456</b>
   <c>789</c>
</in-xml>
return (functx:sort-document-order(
   ($in-xml/c,$in-xml/a)))
