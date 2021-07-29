(:**************************************************************:)
(: Test: functx-functx-first-node-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The XML node in a sequence that appears first in document order 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_first-node.html 
 : @param   $nodes the sequence of nodes 
 :) 
declare function functx:first-node 
  ( $nodes as node()* )  as node()? {
       
   ($nodes/.)[1]
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
return (functx:first-node($in-xml//fName), functx:first-node(
     ($in-xml//lName,
      $in-xml//fName) ))