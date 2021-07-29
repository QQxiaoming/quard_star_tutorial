(:**************************************************************:)
(: Test: functx-functx-non-distinct-values-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Returns any values that appear more than once in a sequence 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_non-distinct-values.html 
 : @param   $seq the sequence of values 
 :) 
declare function functx:non-distinct-values 
  ( $seq as xs:anyAtomicType* )  as xs:anyAtomicType* {
       
   for $val in distinct-values($seq)
   return $val[count($seq[. = $val]) > 1]
 } ;
(functx:non-distinct-values( (1,2,1,3)), functx:non-distinct-values( (1,2,1,3,2.0)), functx:non-distinct-values( (1,2,3) ))