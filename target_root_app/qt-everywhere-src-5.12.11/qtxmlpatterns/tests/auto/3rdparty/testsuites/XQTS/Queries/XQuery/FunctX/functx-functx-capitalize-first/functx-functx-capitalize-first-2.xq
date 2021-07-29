(:**************************************************************:)
(: Test: functx-functx-capitalize-first-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Capitalizes the first character of a string 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_capitalize-first.html 
 : @param   $arg the word or phrase to capitalize 
 :) 
declare function functx:capitalize-first 
  ( $arg as xs:string? )  as xs:string? {
       
   concat(upper-case(substring($arg,1,1)),
             substring($arg,2))
 } ;
(functx:capitalize-first('hello world'))
