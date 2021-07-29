(: Name: constprolog-19 :)
(: Description: Evaluation of a prolog with construction declaration set to "strip" for computed element and :)
(: used in boolean expression ("or" and "fn:false()") :)

declare construction strip;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := element someElement{"some content"}
return
 ($anElement instance of element(*,xs:untyped)) or fn:false()