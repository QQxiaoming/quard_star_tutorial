(:**************************************************************:)
(: Test: functx-functx-id-from-element-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(:~
 : Gets the ID of an XML element 
 :
 : @author  Priscilla Walmsley, Datypic 
 : @version 1.0 
 : @see     http://www.xqueryfunctions.com/xq/functx_id-from-element.html 
 : @param   $element the element 
 :) 
declare function functx:id-from-element 
  ( $element as element()? )  as xs:string? {
       
  data(($element/@*[id(.) is ..])[1])
 } ;

let $book := doc($input-context1)
return (functx:id-from-element($book/book/section[1]))
