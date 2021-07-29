(:**************************************************************:)
(: Test: functx-functx-contains-word-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether one string contains another, as a separate word 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_contains-word.html 
 : @param   $arg the string to search 
 : @param   $word the word to find 
 :) 
declare function functx:contains-word 
  ( $arg as xs:string? ,
    $word as xs:string )  as xs:boolean {
       
   matches(upper-case($arg),
           concat('^(.*\W)?',
                     upper-case(functx:escape-for-regex($word)),
                     '(\W.*)?$'))
 } ;

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
(functx:contains-word('abc def ghi', 'abc'))
