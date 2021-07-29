(:**************************************************************:)
(: Test: functx-functx-value-intersect-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The intersection of two sequences of values 
 :
 : @author  W3C XML Query Working Group 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_value-intersect.html 
 : @param   $arg1 the first sequence 
 : @param   $arg2 the second sequence 
 :) 
declare function functx:value-intersect 
  ( $arg1 as xs:anyAtomicType* ,
    $arg2 as xs:anyAtomicType* )  as xs:anyAtomicType* {
       
  distinct-values($arg1[.=$arg2])
 } ;
(functx:value-intersect((1,2,2,3),(2,3)))
