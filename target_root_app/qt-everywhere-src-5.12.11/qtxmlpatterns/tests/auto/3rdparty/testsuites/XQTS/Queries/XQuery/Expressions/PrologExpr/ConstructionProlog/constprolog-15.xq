(: Name: constprolog-15 :)
(: Description: Evaluation of a prolog with construction declaration set to "strip" for computed element and :)
(: used in boolean expression ("and" and "fn:true()") :)

declare construction strip;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := element someElement{"some content"}
return
 ($anElement instance of element(*,xs:untyped)) and fn:true()