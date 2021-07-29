(:**************************************************************:)
(: Test: functx-functx-sequence-type-7                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The built-in type of an atomic value 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_atomic-type.html 
 : @param   $values the value(s) whose type you want to determine 
 :) 
declare function functx:atomic-type 
  ( $values as xs:anyAtomicType* )  as xs:string* {
       
 for $val in $values
 return
 (if ($val instance of xs:untypedAtomic) then 'xs:untypedAtomic'
 else if ($val instance of xs:anyURI) then 'xs:anyURI'
 else if ($val instance of xs:ENTITY) then 'xs:ENTITY'
 else if ($val instance of xs:ID) then 'xs:ID'
 else if ($val instance of xs:NMTOKEN) then 'xs:NMTOKEN'
 else if ($val instance of xs:language) then 'xs:language'
 else if ($val instance of xs:NCName) then 'xs:NCName'
 else if ($val instance of xs:Name) then 'xs:Name'
 else if ($val instance of xs:token) then 'xs:token'
 else if ($val instance of xs:normalizedString)
         then 'xs:normalizedString'
 else if ($val instance of xs:string) then 'xs:string'
 else if ($val instance of xs:QName) then 'xs:QName'
 else if ($val instance of xs:boolean) then 'xs:boolean'
 else if ($val instance of xs:base64Binary) then 'xs:base64Binary'
 else if ($val instance of xs:hexBinary) then 'xs:hexBinary'
 else if ($val instance of xs:byte) then 'xs:byte'
 else if ($val instance of xs:short) then 'xs:short'
 else if ($val instance of xs:int) then 'xs:int'
 else if ($val instance of xs:long) then 'xs:long'
 else if ($val instance of xs:unsignedByte) then 'xs:unsignedByte'
 else if ($val instance of xs:unsignedShort) then 'xs:unsignedShort'
 else if ($val instance of xs:unsignedInt) then 'xs:unsignedInt'
 else if ($val instance of xs:unsignedLong) then 'xs:unsignedLong'
 else if ($val instance of xs:positiveInteger)
         then 'xs:positiveInteger'
 else if ($val instance of xs:nonNegativeInteger)
         then 'xs:nonNegativeInteger'
 else if ($val instance of xs:negativeInteger)
         then 'xs:negativeInteger'
 else if ($val instance of xs:nonPositiveInteger)
         then 'xs:nonPositiveInteger'
 else if ($val instance of xs:integer) then 'xs:integer'
 else if ($val instance of xs:decimal) then 'xs:decimal'
 else if ($val instance of xs:float) then 'xs:float'
 else if ($val instance of xs:double) then 'xs:double'
 else if ($val instance of xs:date) then 'xs:date'
 else if ($val instance of xs:time) then 'xs:time'
 else if ($val instance of xs:dateTime) then 'xs:dateTime'
 else if ($val instance of xs:dayTimeDuration)
         then 'xs:dayTimeDuration'
 else if ($val instance of xs:yearMonthDuration)
         then 'xs:yearMonthDuration'
 else if ($val instance of xs:duration) then 'xs:duration'
 else if ($val instance of xs:gMonth) then 'xs:gMonth'
 else if ($val instance of xs:gYear) then 'xs:gYear'
 else if ($val instance of xs:gYearMonth) then 'xs:gYearMonth'
 else if ($val instance of xs:gDay) then 'xs:gDay'
 else if ($val instance of xs:gMonthDay) then 'xs:gMonthDay'
 else 'unknown')
 } ;

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

(:~
 : The sequence type that represents a sequence of nodes or values 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_sequence-type.html 
 : @param   $items the items whose sequence type you want to determine 
 :) 
declare function functx:sequence-type 
  ( $items as item()* )  as xs:string {
       
concat(
  if (empty($items))
  then 'empty-sequence()'
  else if (every $val in $items
           satisfies $val instance of xs:anyAtomicType)
  then if (count(distinct-values(functx:atomic-type($items)))
           > 1)
  then 'xs:anyAtomicType'
  else functx:atomic-type($items[1])
  else if (some $val in $items
           satisfies $val instance of xs:anyAtomicType)
  then 'item()'
  else if (count(distinct-values(functx:node-kind($items))) > 1)
  then 'node()'
  else concat(functx:node-kind($items[1]),'()')
  ,
  if (count($items) > 1)
  then '+' else '')
   } ;

let $in-xml := <in-xml>
  <a att1="y">x</a>
  <b att1="x">x</b>
  <!-- comment -->
</in-xml>
return (functx:sequence-type($in-xml/*/@*))
