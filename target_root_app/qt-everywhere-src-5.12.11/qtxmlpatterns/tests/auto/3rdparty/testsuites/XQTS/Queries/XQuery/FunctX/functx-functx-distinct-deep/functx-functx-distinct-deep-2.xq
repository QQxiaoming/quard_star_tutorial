(:**************************************************************:)
(: Test: functx-functx-distinct-deep-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The XML nodes with distinct values, taking into account attributes and descendants 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_distinct-deep.html 
 : @param   $nodes the sequence of nodes to test 
 :) 
declare function functx:distinct-deep 
  ( $nodes as node()* )  as node()* {
       
    for $seq in (1 to count($nodes))
    return $nodes[$seq][not(functx:is-node-in-sequence-deep-equal(
                          .,$nodes[position() < $seq]))]
 } ;

(:~
 : Whether an XML node is in a sequence, based on contents and attributes 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-node-in-sequence-deep-equal.html 
 : @param   $node the node to test 
 : @param   $seq the sequence of nodes to search 
 :) 
declare function functx:is-node-in-sequence-deep-equal 
  ( $node as node()? ,
    $seq as node()* )  as xs:boolean {
       
   some $nodeInSeq in $seq satisfies deep-equal($nodeInSeq,$node)
 } ;

let $in-xml := <authors>
   <author>
      <fName>Kate</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName>Kate</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName>Kate</fName>
      <lName>Doe</lName>
   </author>
</authors>
return (functx:distinct-deep($in-xml//lName))
