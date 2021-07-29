(:**************************************************************:)
(: Test: functx-functx-max-determine-type-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The maximum value in a sequence, figuring out its type (numeric or string) 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_max-determine-type.html 
 : @param   $seq the sequence of values to test 
 :) 
declare function functx:max-determine-type 
  ( $seq as xs:anyAtomicType* )  as xs:anyAtomicType? {
       
   if (every $value in $seq satisfies ($value castable as xs:double))
   then max(for $value in $seq return xs:double($value))
   else max(for $value in $seq return xs:string($value))
 } ;

let $in-xml := <values>
   <nums>
      <num>12</num>
      <num>23</num>
      <num>115</num>
      <num>12.5</num>
   </nums>
   <strings>
      <string>def</string>
      <string>abc</string>
   </strings>
</values>
return (functx:max-determine-type(
   $in-xml//(num|string)))
