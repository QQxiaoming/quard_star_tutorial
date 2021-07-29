(:**************************************************************:)
(: Test: functx-functx-depth-of-node-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The depth (level) of a node in an XML tree 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_depth-of-node.html 
 : @param   $node the node to check 
 :) 
declare function functx:depth-of-node 
  ( $node as node()? )  as xs:integer {
       
   count($node/ancestor-or-self::node())
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
return (functx:depth-of-node($in-xml/author[1]))
