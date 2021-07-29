(:**************************************************************:)
(: Test: functx-functx-is-absolute-uri-5                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(:~
 : Whether a URI is absolute 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_is-absolute-uri.html 
 : @param   $uri the URI to test 
 :) 
declare function functx:is-absolute-uri 
  ( $uri as xs:string? )  as xs:boolean {
       
   matches($uri,'^[a-z]+:')
 } ;
(functx:is-absolute-uri('prod.html'))
