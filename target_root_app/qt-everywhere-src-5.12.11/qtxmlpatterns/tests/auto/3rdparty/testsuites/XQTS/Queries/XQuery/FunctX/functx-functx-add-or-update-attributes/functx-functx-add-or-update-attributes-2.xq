(:**************************************************************:)
(: Test: functx-functx-add-or-update-attributes-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
declare namespace new = "http://new"; 
(:~
 : Adds attributes to XML elements 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_add-or-update-attributes.html 
 : @param   $elements the element(s) to which you wish to add the attribute 
 : @param   $attrNames the name(s) of the attribute(s) to add 
 : @param   $attrValues the value(s) of the attribute(s) to add 
 :) 
declare function functx:add-or-update-attributes 
  ( $elements as element()* ,
    $attrNames as xs:QName* ,
    $attrValues as xs:anyAtomicType* )  as element()? {
       
   for $element in $elements
   return element { node-name($element)}
                  { for $attrName at $seq in $attrNames
                    return attribute {$attrName}
                                     {$attrValues[$seq]},
                    $element/@*[not(node-name(.) = $attrNames)],
                    $element/node() }
 } ;

let $in-xml := <in-xml>
   <a>x</a>
   <b att1="x">x</b>
</in-xml>
return (functx:add-or-update-attributes(
   $in-xml/a,
   (xs:QName('att1'),xs:QName('att2')),
   (1,2)))
