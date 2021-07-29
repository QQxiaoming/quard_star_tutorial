(:**************************************************************:)
(: Test: functx-functx-index-of-deep-equal-node-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The position of a node in a sequence, based on contents and attributes 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_index-of-deep-equal-node.html 
 : @param   $nodes the node sequence 
 : @param   $nodeToFind the node to find in the sequence 
 :) 
declare function functx:index-of-deep-equal-node 
  ( $nodes as node()* ,
    $nodeToFind as node() )  as xs:integer* {
       
  for $seq in (1 to count($nodes))
  return $seq[deep-equal($nodes[$seq],$nodeToFind)]
 } ;

let $in-xml := <authors>
   <author>
      <fName/>
      <lName>Smith</lName>
   </author>
   <author>
      <fName>Kate</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName>John</fName>
      <lName>Doe</lName>
   </author>
</authors>
return 
let $anAuthor := <author>
  <fName>Kate</fName>
  <lName>Jones</lName>
</author>
return 
let $anotherAuthor := <author>
  <fName>John</fName>
  <lName>Smith</lName>
</author>
return (functx:index-of-deep-equal-node(
     $in-xml/author,$anAuthor), functx:index-of-deep-equal-node(
     $in-xml/author,$anotherAuthor), functx:index-of-deep-equal-node(
     $in-xml/author/lName,$anAuthor/lName))