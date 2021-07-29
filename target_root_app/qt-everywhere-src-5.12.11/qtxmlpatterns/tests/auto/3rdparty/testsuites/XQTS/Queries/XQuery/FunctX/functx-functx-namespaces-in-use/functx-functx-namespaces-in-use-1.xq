(:**************************************************************:)
(: Test: functx-functx-namespaces-in-use-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : A list of namespaces used in element/attribute names in an XML fragment 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_namespaces-in-use.html 
 : @param   $root the root node to start from 
 :) 
declare function functx:namespaces-in-use 
  ( $root as node()? )  as xs:anyURI* {
       
   distinct-values(
      $root/descendant-or-self::*/(.|@*)/namespace-uri(.))
 } ;

let $in-xml := <authors xmlns="http://ns.example.com/abc" xmlns:d="http://ns.example.com/def">
   <author xmlns="http://ns.example.com/ghi">
      <fName xmlns:x="http://ns.example.com/xyz" x:attr="123">Kate</fName>
      <lName>Jones</lName>
   </author>
</authors>
return (functx:namespaces-in-use($in-xml))
