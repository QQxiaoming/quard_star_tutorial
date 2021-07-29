(:**************************************************************:)
(: Test: functx-functx-id-untyped-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Gets XML element(s) that have an attribute with a particular value 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_id-untyped.html 
 : @param   $node the root node(s) to start from 
 : @param   $id the "id" to find 
 :) 
declare function functx:id-untyped 
  ( $node as node()* ,
    $id as xs:anyAtomicType )  as element()* {
       
  $node//*[@* = $id]
 } ;

let $in-xml := <in-xml>
  <a id="A001">abc</a>
  <b foo="A001">def</b>
  <c id="B001">ghi</c>
</in-xml>
  
return (functx:id-untyped($in-xml,'C001'))
