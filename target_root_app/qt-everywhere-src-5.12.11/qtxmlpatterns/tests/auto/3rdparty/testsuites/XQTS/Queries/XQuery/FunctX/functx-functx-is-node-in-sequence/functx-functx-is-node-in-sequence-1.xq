(:**************************************************************:)
(: Test: functx-functx-is-node-in-sequence-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
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

let $in-xml := <prices>
   <price value="29.99" discount="10.00"/>
   <price value="39.99" discount="6.00"/>
   <price value="49.99" discount=""/>
</prices>
return 
let $aPrice := <price value="49.99" discount=""/>
return (functx:is-node-in-sequence(
     $in-xml/price[1],$in-xml/price))
