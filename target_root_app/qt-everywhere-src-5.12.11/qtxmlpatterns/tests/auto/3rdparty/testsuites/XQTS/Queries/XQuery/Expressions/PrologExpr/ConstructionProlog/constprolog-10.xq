(: Name: constprolog-10 :)
(: Description: Evaluation of a prolog with construction declaration set to "preserve" for a directly constructed element and used as argument to fn:not.:)

declare construction preserve;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := <someElement>some content</someElement>
return
   fn:not($anElement instance of element(*,xs:anyType))