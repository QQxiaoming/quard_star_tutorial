(:**************************************************************:)
(: Test: functx-functx-words-to-camel-case-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Capitalizes the first character of a string 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_capitalize-first.html 
 : @param   $arg the word or phrase to capitalize 
 :) 
declare function functx:capitalize-first 
  ( $arg as xs:string? )  as xs:string? {
       
   concat(upper-case(substring($arg,1,1)),
             substring($arg,2))
 } ;

(:~
 : Turns a string of words into camelCase 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_words-to-camel-case.html 
 : @param   $arg the string to modify 
 :) 
declare function functx:words-to-camel-case 
  ( $arg as xs:string? )  as xs:string {
       
     string-join((tokenize($arg,'\s+')[1],
       for $word in tokenize($arg,'\s+')[position() > 1]
       return functx:capitalize-first($word))
      ,'')
 } ;
(functx:words-to-camel-case('this Is A Term'))
