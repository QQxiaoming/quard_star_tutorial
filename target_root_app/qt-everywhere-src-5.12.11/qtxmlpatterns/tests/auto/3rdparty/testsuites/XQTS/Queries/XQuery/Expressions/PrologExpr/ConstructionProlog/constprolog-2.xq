(: Name: constprolog-2 :)
(: Description: Evaluation of a prolog with construction declaration set to "strip" and used with directly construted element.:)

declare construction strip;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $anElement := <anElement>some content</anElement>
return
   $anElement instance of element(*,xs:untyped)