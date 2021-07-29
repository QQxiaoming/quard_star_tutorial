(:**************************************************************:)
(: Test: functx-functx-open-ref-document-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Resolves a relative URI and references it, returning an XML document 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_open-ref-document.html 
 : @param   $refNode a node whose value is a relative URI reference 
 :) 
declare function functx:open-ref-document 
  ( $refNode as node() )  as document-node() {
       
   if (base-uri($refNode))
   then doc(resolve-uri($refNode, base-uri($refNode)))
   else doc(resolve-uri($refNode))
 } ;

let $in-xml := <a href="temp/other.html">Some other file</a>
return 
let $in-xml2 := <a href="../../../../temp/dummy.xml">Dummy file</a>
return (functx:open-ref-document($in-xml2/@href))