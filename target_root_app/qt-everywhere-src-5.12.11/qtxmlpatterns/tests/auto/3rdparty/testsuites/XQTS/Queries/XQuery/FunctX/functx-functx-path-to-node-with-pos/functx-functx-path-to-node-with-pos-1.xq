(:**************************************************************:)
(: Test: functx-functx-path-to-node-with-pos-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The position of a node in a sequence, based on node identity 
 :
 : @author  W3C XML Query Working Group 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_index-of-node.html 
 : @param   $nodes the node sequence 
 : @param   $nodeToFind the node to find in the sequence 
 :) 
declare function functx:index-of-node 
  ( $nodes as node()* ,
    $nodeToFind as node() )  as xs:integer* {
       
  for $seq in (1 to count($nodes))
  return $seq[$nodes[$seq] is $nodeToFind]
 } ;

(:~
 : A unique path to an XML node (or sequence of nodes) 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_path-to-node-with-pos.html 
 : @param   $node the node sequence 
 :) 
declare function functx:path-to-node-with-pos 
  ( $node as node()? )  as xs:string {
       
string-join(
  for $ancestor in $node/ancestor-or-self::*
  let $sibsOfSameName := $ancestor/../*[name() = name($ancestor)]
  return concat(name($ancestor),
   if (count($sibsOfSameName) <= 1)
   then ''
   else concat(
      '[',functx:index-of-node($sibsOfSameName,$ancestor),']'))
 , '/')
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
return (functx:path-to-node-with-pos(
     $in-xml//lName[. = 'Doe']))
