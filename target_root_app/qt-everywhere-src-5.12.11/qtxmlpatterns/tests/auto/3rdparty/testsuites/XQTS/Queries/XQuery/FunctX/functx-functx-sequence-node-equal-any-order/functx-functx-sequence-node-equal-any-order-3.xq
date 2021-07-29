(:**************************************************************:)
(: Test: functx-functx-sequence-node-equal-any-order-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether two sequences contain the same XML nodes, regardless of order 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_sequence-node-equal-any-order.html 
 : @param   $seq1 the first sequence of nodes 
 : @param   $seq2 the second sequence of nodes 
 :) 
declare function functx:sequence-node-equal-any-order 
  ( $seq1 as node()* ,
    $seq2 as node()* )  as xs:boolean {
       
  not( ($seq1 except $seq2, $seq2 except $seq1))
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
   <fName>Kate</fName>
   <lName>Jones</lName>
</author>
return (functx:sequence-node-equal-any-order(
     $in-xml/author[1],$anAuthor))
