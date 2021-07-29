(:**************************************************************:)
(: Test: functx-functx-update-attributes-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Updates the attribute value of an XML element 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_update-attributes.html 
 : @param   $elements the element(s) for which you wish to update the attribute 
 : @param   $attrNames the name(s) of the attribute(s) to add 
 : @param   $attrValues the value(s) of the attribute(s) to add 
 :) 
declare function functx:update-attributes 
  ( $elements as element()* ,
    $attrNames as xs:QName* ,
    $attrValues as xs:anyAtomicType* )  as element()? {
       
   for $element in $elements
   return element { node-name($element)}
                  { for $attrName at $seq in $attrNames
                    return if ($element/@*[node-name(.) = $attrName])
                           then attribute {$attrName}
                                     {$attrValues[$seq]}
                           else (),
                    $element/@*[not(node-name(.) = $attrNames)],
                    $element/node() }
 } ;

let $in-xml := <in-xml xmlns:new='http://new'>
   <a att1='def'>x</a>
   <b>x</b>
   <c new:att1='def'>x</c>
</in-xml>
return (functx:update-attributes(
   $in-xml/a, xs:QName('att1'), 123), functx:update-attributes(
   $in-xml/a,
   (xs:QName('att1'),xs:QName('att2')),
   (1,2)), functx:update-attributes(
   $in-xml/b, xs:QName('att1'), 123), functx:update-attributes(
     $in-xml/c,
     QName('http://new','prefix:att1'),
     123))