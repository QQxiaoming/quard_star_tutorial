(: Name: Constr-inscope-9 :)
(: Written by: Andreas Behm :)
(: Description: copy element node with namespace :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new>{$input-context//*:child1}</new>