(:**************************************************************:)
(: Test: functx-functx-exclusive-or-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether one (and only one) of two boolean values is true 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_exclusive-or.html 
 : @param   $arg1 the first boolean value 
 : @param   $arg2 the second boolean value 
 :) 
declare function functx:exclusive-or 
  ( $arg1 as xs:boolean? ,
    $arg2 as xs:boolean? )  as xs:boolean? {
       
   $arg1 != $arg2
 } ;
(functx:exclusive-or(true(),false()))
