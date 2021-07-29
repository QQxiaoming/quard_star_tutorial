(:**************************************************************:)
(: Test: functx-functx-copy-attributes-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Copies attributes from one element to another 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_copy-attributes.html 
 : @param   $copyTo the element to copy attributes to 
 : @param   $copyFrom the element to copy attributes from 
 :) 
declare function functx:copy-attributes 
  ( $copyTo as element() ,
    $copyFrom as element() )  as element() {
       
   element { node-name($copyTo)}
           { $copyTo/@*[not(node-name(.) = $copyFrom/@*/node-name(.))],
             $copyFrom/@*,
             $copyTo/node() }

 } ;

let $in-xml := <in-xml>
   <a>123</a>>
   <b x="1" y="2">456</b>
   <c x="9">123</c>
   <d z="5">123</d>
</in-xml>
return (functx:copy-attributes(
   $in-xml/a, $in-xml/b), functx:copy-attributes(
   $in-xml/b, $in-xml/c), functx:copy-attributes(
   $in-xml/d, $in-xml/c))