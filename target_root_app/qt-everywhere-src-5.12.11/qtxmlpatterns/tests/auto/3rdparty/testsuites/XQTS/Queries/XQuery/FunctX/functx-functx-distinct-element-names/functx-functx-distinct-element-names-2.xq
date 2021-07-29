(:**************************************************************:)
(: Test: functx-functx-distinct-element-names-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The distinct names of all elements in an XML fragment 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_distinct-element-names.html 
 : @param   $nodes the root(s) to start from 
 :) 
declare function functx:distinct-element-names 
  ( $nodes as node()* )  as xs:string* {
       
   distinct-values($nodes/descendant-or-self::*/name(.))
 } ;

(:~
 : A path to an XML node (or sequence of nodes) 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_path-to-node.html 
 : @param   $nodes the node sequence 
 :) 
declare function functx:path-to-node 
  ( $nodes as node()* )  as xs:string* {
       
$nodes/string-join(ancestor-or-self::*/name(.), '/')
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
return (functx:distinct-element-names(
   $in-xml/author[1]))
