(: Name: Constr-inscope-1 :)
(: Written by: Andreas Behm :)
(: Description: Rename inscope namespace :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new xmlns:foo="http://www.example.com">{$input-context//@*:attr1}</new>
