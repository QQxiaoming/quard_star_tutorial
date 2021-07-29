(:**************************************************************:)
(: Test: functx-functx-substring-before-last-match-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The substring before the last text that matches a regex 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_substring-before-last-match.html 
 : @param   $arg the string to substring 
 : @param   $regex the regular expression 
 :) 
declare function functx:substring-before-last-match 
  ( $arg as xs:string? ,
    $regex as xs:string )  as xs:string? {
       
   replace($arg,concat('^(.*)',$regex,'.*'),'$1')
 } ;
(functx:substring-before-last-match(
  'abc-def-ghi', '[ce]'), functx:substring-before-last-match(
  'abcd-abcd', 'ab?'), functx:substring-before-last-match(
  'abcd-abcd', 'x'))