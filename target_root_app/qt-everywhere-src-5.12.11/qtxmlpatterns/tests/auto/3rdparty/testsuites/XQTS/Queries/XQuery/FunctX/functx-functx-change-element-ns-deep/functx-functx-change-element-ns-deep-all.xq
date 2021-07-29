(:**************************************************************:)
(: Test: functx-functx-change-element-ns-deep-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Changes the namespace of XML elements and its descendants 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_change-element-ns-deep.html 
 : @param   $nodes the nodes to change 
 : @param   $newns the new namespace 
 : @param   $prefix the prefix to use for the new namespace 
 :) 
declare function functx:change-element-ns-deep 
  ( $nodes as node()* ,
    $newns as xs:string ,
    $prefix as xs:string )  as node()* {
       
  for $node in $nodes
  return if ($node instance of element())
         then (element
               {QName ($newns,
                          concat($prefix,
                                    if ($prefix = '')
                                    then ''
                                    else ':',
                                    local-name($node)))}
               {$node/@*,
                functx:change-element-ns-deep($node/node(),
                                           $newns, $prefix)})
         else if ($node instance of document-node())
         then functx:change-element-ns-deep($node/node(),
                                           $newns, $prefix)
         else $node
 } ;

let $in-xml := <bar:a xmlns:bar="http://bar">
   <bar:b>557</bar:b>
   <bar:c>xyz</bar:c>
</bar:a>
return (functx:change-element-ns-deep(
     $in-xml, 'http://foo',''), functx:change-element-ns-deep(
     $in-xml, 'http://foo','foo'))