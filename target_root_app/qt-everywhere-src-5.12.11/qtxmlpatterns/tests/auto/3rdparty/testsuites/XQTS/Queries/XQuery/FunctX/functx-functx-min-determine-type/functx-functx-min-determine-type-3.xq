(:**************************************************************:)
(: Test: functx-functx-min-determine-type-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The minimum value in a sequence, figuring out its type (numeric or string) 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_min-determine-type.html 
 : @param   $seq the sequence of values to test 
 :) 
declare function functx:min-determine-type 
  ( $seq as xs:anyAtomicType* )  as xs:anyAtomicType? {
       
   if (every $value in $seq satisfies ($value castable as xs:double))
   then min(for $value in $seq return xs:double($value))
   else min(for $value in $seq return xs:string($value))
 } ;

let $in-xml := <values>
   <nums>
      <num>12</num>
      <num>23</num>
      <num>115</num>
      <num>12.5</num>
   </nums>
   <strings>
      <str>def</str>
      <str>abc</str>
   </strings>
</values>
return (functx:min-determine-type($in-xml//(num|str)))
