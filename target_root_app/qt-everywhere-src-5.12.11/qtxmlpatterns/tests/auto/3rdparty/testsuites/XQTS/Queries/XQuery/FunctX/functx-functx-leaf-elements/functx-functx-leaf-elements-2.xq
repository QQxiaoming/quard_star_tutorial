(:**************************************************************:)
(: Test: functx-functx-leaf-elements-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : All XML elements that don't have any child elements 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_leaf-elements.html 
 : @param   $root the root 
 :) 
declare function functx:leaf-elements 
  ( $root as node()? )  as element()* {
       
   $root/descendant-or-self::*[not(*)]
 } ;

let $in-xml := <authors>
   <author>
      <fName>Kate</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName>John</fName>
      <lName>Doe</lName>
   </author>
</authors>
return (functx:leaf-elements($in-xml/author[1]))
