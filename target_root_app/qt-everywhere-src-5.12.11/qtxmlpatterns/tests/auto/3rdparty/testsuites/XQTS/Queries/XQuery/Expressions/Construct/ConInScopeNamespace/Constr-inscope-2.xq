(: Name: Constr-inscope-2 :)
(: Written by: Andreas Behm :)
(: Description: Rename inscope namespace :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new>{$input-context//@*:attr1, $input-context//@*:attr2}</new>