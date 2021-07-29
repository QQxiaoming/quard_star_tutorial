(: Name: constprolog-14 :)
(: Description: Evaluation of a prolog with construction declaration set to "strip" for direct and :)
(: used in boolean expression ("and" and "fn:true()") :)

declare construction strip;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some content</anElement> 
return
 ($var instance of element(*,xs:untyped)) and fn:true()