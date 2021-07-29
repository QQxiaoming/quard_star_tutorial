(:**************************************************************:)
(: Test: functx-functx-is-ancestor-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether an XML node is an ancestor of another node 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-ancestor.html 
 : @param   $node1 the first node 
 : @param   $node2 the second node 
 :) 
declare function functx:is-ancestor 
  ( $node1 as node() ,
    $node2 as node() )  as xs:boolean {
       
   exists($node1 intersect $node2/ancestor::node())
 } ;

let $in-xml := <authors>
   <author>
      <fName>Kate</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName>John</fName>
      <lName>Doe</lName>
   </author>
</authors>
return (functx:is-ancestor(
     $in-xml//author[1]/fName,
     $in-xml//author[1]/fName/text()))
