(: Name: constprolog-9 :)
(: Description: Evaluation of a prolog with construction declaration set to "strip" for a computed element and used as argument to fn:not.:)

declare construction strip;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := element someElement{"some content"}
return
   fn:not($anElement instance of element(*,xs:untyped))