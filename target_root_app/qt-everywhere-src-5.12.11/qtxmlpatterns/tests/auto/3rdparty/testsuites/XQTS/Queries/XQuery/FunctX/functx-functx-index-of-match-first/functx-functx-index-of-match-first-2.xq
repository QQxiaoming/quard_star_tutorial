(:**************************************************************:)
(: Test: functx-functx-index-of-match-first-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The first position of a matching substring 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_index-of-match-first.html 
 : @param   $arg the string 
 : @param   $pattern the pattern to match 
 :) 
declare function functx:index-of-match-first 
  ( $arg as xs:string? ,
    $pattern as xs:string )  as xs:integer? {
       
  if (matches($arg,$pattern))
  then string-length(tokenize($arg, $pattern)[1]) + 1
  else ()
 } ;
(functx:index-of-match-first(
   'abcdabcdabcd','bcd'))
