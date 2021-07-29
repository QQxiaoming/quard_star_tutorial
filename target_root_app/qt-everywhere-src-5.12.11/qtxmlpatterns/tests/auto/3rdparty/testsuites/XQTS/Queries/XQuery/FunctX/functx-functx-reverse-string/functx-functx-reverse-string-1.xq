(:**************************************************************:)
(: Test: functx-functx-reverse-string-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Reverses the order of characters 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_reverse-string.html 
 : @param   $arg the string to reverse 
 :) 
declare function functx:reverse-string 
  ( $arg as xs:string? )  as xs:string {
       
   codepoints-to-string(reverse(string-to-codepoints($arg)))
 } ;
(functx:reverse-string('abc'))
