(:**************************************************************:)
(: Test: functx-functx-line-count-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The number of lines 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_line-count.html 
 : @param   $arg the string to test 
 :) 
declare function functx:line-count 
  ( $arg as xs:string? )  as xs:integer {
       
   count(functx:lines($arg))
 } ;

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

let $lines := 'a
value
on
many
lines'
return (functx:line-count($lines))
