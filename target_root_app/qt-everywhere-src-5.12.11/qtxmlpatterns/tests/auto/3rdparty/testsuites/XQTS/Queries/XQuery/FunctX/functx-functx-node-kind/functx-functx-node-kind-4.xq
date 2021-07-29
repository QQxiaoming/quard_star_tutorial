(:**************************************************************:)
(: Test: functx-functx-node-kind-4                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The XML node kind (element, attribute, text, etc.) 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_node-kind.html 
 : @param   $nodes the node(s) whose kind you want to determine 
 :) 
declare function functx:node-kind 
  ( $nodes as node()* )  as xs:string* {
       
 for $node in $nodes
 return
 if ($node instance of element()) then 'element'
 else if ($node instance of attribute()) then 'attribute'
 else if ($node instance of text()) then 'text'
 else if ($node instance of document-node()) then 'document-node'
 else if ($node instance of comment()) then 'comment'
 else if ($node instance of processing-instruction())
         then 'processing-instruction'
 else 'unknown'
 } ;

let $in-xml := <in-xml>
   <!-- this is in-xml -->
   <?test see?>
   <a z="2">xyz</a>
 </in-xml>
return (functx:node-kind(
   $in-xml/processing-instruction()))
