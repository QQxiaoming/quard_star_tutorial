(:**************************************************************:)
(: Test: functx-functx-is-value-in-sequence-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether an atomic value appears in a sequence  
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-value-in-sequence.html 
 : @param   $value the atomic value to test 
 : @param   $seq the sequence of values to search 
 :) 
declare function functx:is-value-in-sequence 
  ( $value as xs:anyAtomicType? ,
    $seq as xs:anyAtomicType* )  as xs:boolean {
       
   $value = $seq
 } ;
(functx:is-value-in-sequence(1,(1,2,3)), functx:is-value-in-sequence(5,(1,2,3)), functx:is-value-in-sequence(1.0,(1,2,3)))