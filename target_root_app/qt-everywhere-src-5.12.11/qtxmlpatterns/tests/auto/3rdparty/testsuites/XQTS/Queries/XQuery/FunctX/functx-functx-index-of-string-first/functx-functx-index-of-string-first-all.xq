(:**************************************************************:)
(: Test: functx-functx-index-of-string-first-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The first position of a substring 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_index-of-string-first.html 
 : @param   $arg the string 
 : @param   $substring the substring to find 
 :) 
declare function functx:index-of-string-first 
  ( $arg as xs:string? ,
    $substring as xs:string )  as xs:integer? {
       
  if (contains($arg, $substring))
  then string-length(substring-before($arg, $substring))+1
  else ()
 } ;
(functx:index-of-string-first(
   'abcdabcdabcd','abc'), functx:index-of-string-first(
   'abcd','abc'), functx:index-of-string-first(
   'xxx','abc'))