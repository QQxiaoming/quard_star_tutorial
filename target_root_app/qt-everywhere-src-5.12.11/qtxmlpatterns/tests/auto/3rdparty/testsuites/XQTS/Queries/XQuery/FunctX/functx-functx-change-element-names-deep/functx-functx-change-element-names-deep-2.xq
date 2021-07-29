(:**************************************************************:)
(: Test: functx-functx-change-element-names-deep-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
declare namespace dty = "http://datypic.com"; 
(:~
 : Changes the names of elements in an XML fragment 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_change-element-names-deep.html 
 : @param   $nodes the element(s) to change 
 : @param   $oldNames the sequence of names to change from 
 : @param   $newNames the sequence of names to change to 
 :) 
declare function functx:change-element-names-deep 
  ( $nodes as node()* ,
    $oldNames as xs:QName* ,
    $newNames as xs:QName* )  as node()* {
       
  if (count($oldNames) != count($newNames))
  then error(xs:QName('functx:Different_number_of_names'))
  else
   for $node in $nodes
   return if ($node instance of element())
          then element
                 {functx:if-empty
                    ($newNames[index-of($oldNames,
                                           node-name($node))],
                     node-name($node)) }
                 {$node/@*,
                  functx:change-element-names-deep($node/node(),
                                           $oldNames, $newNames)}
          else if ($node instance of document-node())
          then functx:change-element-names-deep($node/node(),
                                           $oldNames, $newNames)
          else $node
 } ;

(:~
 : The first argument if it is not blank, otherwise the second argument 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_if-empty.html 
 : @param   $arg the node that may be empty 
 : @param   $value the item(s) to use if the node is empty 
 :) 
declare function functx:if-empty 
  ( $arg as item()? ,
    $value as item()* )  as item()* {
       
  if (string($arg) != '')
  then data($arg)
  else $value
 } ;

let $in-xml-1 := <in-xml>
   <a>
      <b>b</b>
      <c>c</c>
   </a>
</in-xml>
return 
let $in-xml-2 := <in-xml xmlns:dty="http://datypic.com">
   <a>
      <dty:b>b</dty:b>
      <c>c</c>
   </a>
</in-xml>
return (functx:change-element-names-deep(
 $in-xml-1,
 (xs:QName('a'),
   xs:QName('b'),xs:QName('c')),
 (xs:QName('x'),
   xs:QName('y'),xs:QName('z'))))
