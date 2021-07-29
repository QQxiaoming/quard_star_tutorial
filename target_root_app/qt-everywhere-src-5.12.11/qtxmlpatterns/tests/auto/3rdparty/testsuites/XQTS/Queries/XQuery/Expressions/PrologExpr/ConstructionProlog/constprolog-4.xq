(: Name: constprolog-4 :)
(: Description: Evaluation of a prolog with construction declaration set to "preserve" and used with directly construted element.:)

declare construction preserve;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := <anElement>some content</anElement>
return
   $anElement instance of element(*,xs:anyType)