(:**************************************************************:)
(: Test: functx-functx-max-line-length-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Split a string into separate lines 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_lines.html 
 : @param   $arg the string to split 
 :) 
declare function functx:lines 
  ( $arg as xs:string? )  as xs:string* {
       
   tokenize($arg, '(\r\n?|\n\r?)')
 } ;

(:~
 : The maximum line length 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_max-line-length.html 
 : @param   $arg the string to test 
 :) 
declare function functx:max-line-length 
  ( $arg as xs:string? )  as xs:integer? {
       
   max(
     for $line in functx:lines($arg)
     return string-length($line))
 } ;

let $lines := 'a
value
on several
lines'
return (functx:max-line-length('a value'), functx:max-line-length($lines))