(:**************************************************************:)
(: Test: functx-functx-path-to-node-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
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
return (functx:path-to-node($in-xml//lName[. = 'Doe']))
