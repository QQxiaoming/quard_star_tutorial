(:**************************************************************:)
(: Test: functx-functx-is-node-in-sequence-deep-equal-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether an XML node is in a sequence, based on contents and attributes 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-node-in-sequence-deep-equal.html 
 : @param   $node the node to test 
 : @param   $seq the sequence of nodes to search 
 :) 
declare function functx:is-node-in-sequence-deep-equal 
  ( $node as node()? ,
    $seq as node()* )  as xs:boolean {
       
   some $nodeInSeq in $seq satisfies deep-equal($nodeInSeq,$node)
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
return 
let $anAuthor := <author>
  <fName>John</fName>
  <lName>Doe</lName>
</author>
return (functx:is-node-in-sequence-deep-equal(
     $in-xml/author[1],$in-xml/author), functx:is-node-in-sequence-deep-equal(
         $anAuthor,$in-xml/author), functx:is-node-in-sequence-deep-equal(
     $in-xml/author[1],$in-xml))