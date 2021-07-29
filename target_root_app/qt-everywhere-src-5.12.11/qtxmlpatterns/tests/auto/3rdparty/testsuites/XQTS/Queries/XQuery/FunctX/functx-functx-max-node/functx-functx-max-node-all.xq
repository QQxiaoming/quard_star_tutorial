(:**************************************************************:)
(: Test: functx-functx-max-node-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The XML node whose typed value is the maximum 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_max-node.html 
 : @param   $nodes the sequence of nodes to test 
 :) 
declare function functx:max-node 
  ( $nodes as node()* )  as node()* {
       
   $nodes[. = max($nodes)]
 } ;

let $in-xml := <values>
      <int>1</int>
      <int>23</int>
      <int>115</int>
</values>
return (functx:max-node($in-xml//int))