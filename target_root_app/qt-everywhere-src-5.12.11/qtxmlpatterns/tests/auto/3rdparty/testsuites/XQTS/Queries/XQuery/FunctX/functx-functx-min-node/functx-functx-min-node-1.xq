(:**************************************************************:)
(: Test: functx-functx-min-node-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The XML node whose typed value is the minimum 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_min-node.html 
 : @param   $nodes the sequence of nodes to test 
 :) 
declare function functx:min-node 
  ( $nodes as node()* )  as node()* {
       
   $nodes[. = min($nodes)]
 } ;

let $in-xml := <values>
      <int>1</int>
      <int>23</int>
      <int>115</int>
</values>
return (functx:min-node($in-xml//int))
