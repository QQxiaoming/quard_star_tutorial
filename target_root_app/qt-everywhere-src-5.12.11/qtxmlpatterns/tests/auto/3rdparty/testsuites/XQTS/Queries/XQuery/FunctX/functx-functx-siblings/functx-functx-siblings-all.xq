(:**************************************************************:)
(: Test: functx-functx-siblings-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : The siblings of an XML node 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_siblings.html 
 : @param   $node the node 
 :) 
declare function functx:siblings 
  ( $node as node()? )  as node()* {
       
   $node/../node() except $node
 } ;

let $in-xml := <authors a1='xyz'>
   <author a2='abc'>
      <fName a3='def'>Kate</fName>
      <fName a3='def'>Jane</fName>
      <lName>Jones</lName>
   </author>
   <author>
      <fName a3='def'>John</fName>
      <lName>Doe</lName>
   </author>
</authors>
return (functx:siblings(
     $in-xml/author[1]), functx:siblings(
     $in-xml/author[1]/fName[2]))