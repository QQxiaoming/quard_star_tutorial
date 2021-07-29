(:**************************************************************:)
(: Test: functx-functx-change-element-ns-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Changes the namespace of XML elements 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_change-element-ns.html 
 : @param   $elements the elements to change 
 : @param   $newns the new namespace 
 : @param   $prefix the prefix to use for the new namespace 
 :) 
declare function functx:change-element-ns 
  ( $elements as element()* ,
    $newns as xs:string ,
    $prefix as xs:string )  as element()? {
       
   for $element in $elements
   return
   element {QName ($newns,
                      concat($prefix,
                                if ($prefix = '')
                                then ''
                                else ':',
                                local-name($element)))}
           {$element/@*, $element/node()}
 } ;

let $in-xml := <bar:a xmlns:bar="http://bar">
   <bar:b>557</bar:b>
   <bar:c>xyz</bar:c>
</bar:a>
return (functx:change-element-ns(
     $in-xml, 'http://foo','foo'))
