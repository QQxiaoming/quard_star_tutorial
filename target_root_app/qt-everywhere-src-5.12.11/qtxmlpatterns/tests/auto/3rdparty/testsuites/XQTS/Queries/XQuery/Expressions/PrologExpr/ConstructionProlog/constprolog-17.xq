(: Name: constprolog-17 :)
(: Description: Evaluation of a prolog with construction declaration set to "preserve" for computed element and :)
(: used in boolean expression ("and" and "fn:true()") :)

declare construction preserve;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := element someElement{"some content"}
return
 ($anElement instance of element(*,xs:anyType)) and fn:true()