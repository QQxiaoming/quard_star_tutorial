(:**************************************************************:)
(: Test: functx-functx-min-non-empty-string-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The minimum of a sequence of strings, ignoring "empty" values 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_min-non-empty-string.html 
 : @param   $strings the sequence of strings to search 
 :) 
declare function functx:min-non-empty-string 
  ( $strings as xs:string* )  as xs:string? {
       
   min($strings[. != ''])
 } ;

let $in-xml := <authors>
   <author>
      <fName/>
      <lName>Smith</lName>
   </author>
   <author>
      <fName>Kate</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName>John</fName>
      <lName>Doe</lName>
   </author>
</authors>
return (functx:min-non-empty-string( $in-xml//fName ))
