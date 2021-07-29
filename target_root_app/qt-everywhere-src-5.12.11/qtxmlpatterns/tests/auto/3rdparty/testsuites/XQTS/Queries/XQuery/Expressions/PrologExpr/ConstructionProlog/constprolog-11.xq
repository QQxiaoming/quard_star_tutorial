(: Name: constprolog-11 :)
(: Description: Evaluation of a prolog with construction declaration set to "preserve" for a computed element and used as argument to fn:not.:)

declare construction preserve;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := element someElement{"some content"}
return
   fn:not($anElement instance of element(*,xs:anyType))