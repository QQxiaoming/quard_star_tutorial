(: Name: constprolog-16 :)
(: Description: Evaluation of a prolog with construction declaration set to "preserve" for direct element and :)
(: used in boolean expression ("and" and "fn:true()") :)

declare construction preserve;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := <someElement>content</someElement>
return
 ($anElement instance of element(*,xs:anyType)) and fn:true()