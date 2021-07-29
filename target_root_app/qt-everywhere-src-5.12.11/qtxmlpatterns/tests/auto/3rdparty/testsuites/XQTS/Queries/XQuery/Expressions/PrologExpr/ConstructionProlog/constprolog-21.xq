(: Name: constprolog-21 :)
(: Description: Evaluation of a prolog with construction declaration set to "preserve" for computed element and :)
(: used in boolean expression ("or" and "fn:false()") :)

declare construction preserve;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := element someElement{"some content"}
return
 ($anElement instance of element(*,xs:anyType)) or fn:false()