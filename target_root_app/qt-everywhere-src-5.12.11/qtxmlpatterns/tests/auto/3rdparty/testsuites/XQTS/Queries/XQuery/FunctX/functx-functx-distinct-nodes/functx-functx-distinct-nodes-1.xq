(:**************************************************************:)
(: Test: functx-functx-distinct-nodes-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The distinct XML nodes in a sequence (by node identity) 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_distinct-nodes.html 
 : @param   $nodes the node sequence 
 :) 
declare function functx:distinct-nodes 
  ( $nodes as node()* )  as node()* {
       
    for $seq in (1 to count($nodes))
    return $nodes[$seq][not(functx:is-node-in-sequence(
                                .,$nodes[position() < $seq]))]
 } ;

(:~
 : Whether an XML node is in a sequence, based on node identity 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-node-in-sequence.html 
 : @param   $node the node to test 
 : @param   $seq the sequence of nodes to search 
 :) 
declare function functx:is-node-in-sequence 
  ( $node as node()? ,
    $seq as node()* )  as xs:boolean {
       
   some $nodeInSeq in $seq satisfies $nodeInSeq is $node
 } ;

let $in-xml := <test>
  <child>1</child>
  <child>2</child>
  <child>3</child>
  <child>3</child>
</test>
return (functx:distinct-nodes(
     ($in-xml/child, $in-xml/*) ))
