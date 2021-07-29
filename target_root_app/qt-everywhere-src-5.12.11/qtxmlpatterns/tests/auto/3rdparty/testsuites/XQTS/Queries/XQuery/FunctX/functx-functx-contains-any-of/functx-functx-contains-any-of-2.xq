(:**************************************************************:)
(: Test: functx-functx-contains-any-of-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether a string contains any of a sequence of strings 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_contains-any-of.html 
 : @param   $arg the string to test 
 : @param   $searchStrings the strings to look for 
 :) 
declare function functx:contains-any-of 
  ( $arg as xs:string? ,
    $searchStrings as xs:string* )  as xs:boolean {
       
   some $searchString in $searchStrings
   satisfies contains($arg,$searchString)
 } ;
(functx:contains-any-of('abc',('de','xy')))
