(:**************************************************************:)
(: Test: functx-functx-trim-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Trims leading and trailing whitespace 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_trim.html 
 : @param   $arg the string to trim 
 :) 
declare function functx:trim 
  ( $arg as xs:string? )  as xs:string {
       
   replace(replace($arg,'\s+$',''),'^\s+','')
 } ;
(functx:trim('x   xyz  '))
