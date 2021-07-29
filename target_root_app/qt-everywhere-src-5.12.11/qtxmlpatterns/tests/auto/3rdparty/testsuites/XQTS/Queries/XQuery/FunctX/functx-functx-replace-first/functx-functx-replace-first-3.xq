(:**************************************************************:)
(: Test: functx-functx-replace-first-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Replaces the first match of a pattern 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_replace-first.html 
 : @param   $arg the entire string to change 
 : @param   $pattern the pattern of characters to replace 
 : @param   $replacement the replacement string 
 :) 
declare function functx:replace-first 
  ( $arg as xs:string? ,
    $pattern as xs:string ,
    $replacement as xs:string )  as xs:string {
       
   replace($arg, concat('(^.*?)', $pattern),
             concat('$1',$replacement))
 } ;
(functx:replace-first(
   'elementary', 'e.*?e', 'x'))
