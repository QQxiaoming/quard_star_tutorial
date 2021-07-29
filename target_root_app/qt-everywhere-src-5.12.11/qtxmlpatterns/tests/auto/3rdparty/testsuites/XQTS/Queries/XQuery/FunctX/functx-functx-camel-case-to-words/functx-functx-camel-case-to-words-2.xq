(:**************************************************************:)
(: Test: functx-functx-camel-case-to-words-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Turns a camelCase string into space-separated words 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_camel-case-to-words.html 
 : @param   $arg the string to modify 
 : @param   $delim the delimiter for the words (e.g. a space) 
 :) 
declare function functx:camel-case-to-words 
  ( $arg as xs:string? ,
    $delim as xs:string )  as xs:string {
       
   concat(substring($arg,1,1),
             replace(substring($arg,2),'(\p{Lu})',
                        concat($delim, '$1')))
 } ;
(functx:camel-case-to-words(
     'thisIsACamelCaseTerm',','))
