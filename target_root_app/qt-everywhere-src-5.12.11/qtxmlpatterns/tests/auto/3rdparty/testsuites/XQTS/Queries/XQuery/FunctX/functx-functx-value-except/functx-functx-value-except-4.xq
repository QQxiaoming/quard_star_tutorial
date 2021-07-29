(:**************************************************************:)
(: Test: functx-functx-value-except-4                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The values in one sequence that aren't in another sequence 
 :
 : @author  W3C XML Query Working Group 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_value-except.html 
 : @param   $arg1 the first sequence 
 : @param   $arg2 the second sequence 
 :) 
declare function functx:value-except 
  ( $arg1 as xs:anyAtomicType* ,
    $arg2 as xs:anyAtomicType* )  as xs:anyAtomicType* {
       
  distinct-values($arg1[not(.=$arg2)])
 } ;
(functx:value-except((1,2,2,3),()))
