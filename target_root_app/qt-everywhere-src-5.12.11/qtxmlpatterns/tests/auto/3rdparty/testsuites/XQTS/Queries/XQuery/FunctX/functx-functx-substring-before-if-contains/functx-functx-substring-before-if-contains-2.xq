(:**************************************************************:)
(: Test: functx-functx-substring-before-if-contains-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Performs substring-before, returning the entire string if it does not contain the delimiter 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_substring-before-if-contains.html 
 : @param   $arg the string to substring 
 : @param   $delim the delimiter 
 :) 
declare function functx:substring-before-if-contains 
  ( $arg as xs:string? ,
    $delim as xs:string )  as xs:string? {
       
   if (contains($arg,$delim))
   then substring-before($arg,$delim)
   else $arg
 } ;
(functx:substring-before-if-contains('abcd','x'))
