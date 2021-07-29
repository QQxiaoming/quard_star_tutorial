(: Name: constprolog-3 :)
(: Description: Evaluation of a prolog with construction declaration set to "strip" and used with computed delement.:)

declare construction strip;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := element anElement {"someContent"}
return
   $anElement instance of element(*,xs:untyped)