(:**************************************************************:)
(: Test: functx-functx-substring-before-last-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Escapes regex special characters 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_escape-for-regex.html 
 : @param   $arg the string to escape 
 :) 
declare function functx:escape-for-regex 
  ( $arg as xs:string? )  as xs:string {
       
   replace($arg,
           '(\.|\[|\]|\\|\||\-|\^|\$|\?|\*|\+|\{|\}|\(|\))','\\$1')
 } ;

(:~
 : The substring before the last occurrence of a delimiter 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_substring-before-last.html 
 : @param   $arg the string to substring 
 : @param   $delim the delimiter 
 :) 
declare function functx:substring-before-last 
  ( $arg as xs:string? ,
    $delim as xs:string )  as xs:string {
       
   if (matches($arg, functx:escape-for-regex($delim)))
   then replace($arg,
            concat('^(.*)', functx:escape-for-regex($delim),'.*'),
            '$1')
   else ''
 } ;
(functx:substring-before-last('abcd-abcd', 'ab'))
