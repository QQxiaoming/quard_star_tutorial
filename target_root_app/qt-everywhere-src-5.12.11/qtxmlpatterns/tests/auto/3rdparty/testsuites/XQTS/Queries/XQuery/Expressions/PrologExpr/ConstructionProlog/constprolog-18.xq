(: Name: constprolog-18 :)
(: Description: Evaluation of a prolog with construction declaration set to "strip" for direct and :)
(: used in boolean expression ("or" and "fn:false()") :)

declare construction strip;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some content</anElement> 
return
 ($var instance of element(*,xs:untyped)) or fn:false()