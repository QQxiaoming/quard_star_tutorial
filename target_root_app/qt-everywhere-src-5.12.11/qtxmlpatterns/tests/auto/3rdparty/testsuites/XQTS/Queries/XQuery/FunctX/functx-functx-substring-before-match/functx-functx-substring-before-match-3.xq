(:**************************************************************:)
(: Test: functx-functx-substring-before-match-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The substring before the first text that matches a regex 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_substring-before-match.html 
 : @param   $arg the string to substring 
 : @param   $regex the regular expression 
 :) 
declare function functx:substring-before-match 
  ( $arg as xs:string? ,
    $regex as xs:string )  as xs:string? {
       
   tokenize($arg,$regex)[1]
 } ;
(functx:substring-before-match(
     'abcd-abcd', 'x'))
