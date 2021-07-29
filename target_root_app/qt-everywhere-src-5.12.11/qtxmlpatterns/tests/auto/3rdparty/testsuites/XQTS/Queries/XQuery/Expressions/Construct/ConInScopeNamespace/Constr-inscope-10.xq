(: Name: Constr-inscope-10 :)
(: Written by: Andreas Behm :)
(: Description: copy element node with namespace :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new xmlns="http://www.example.com">{$input-context//*:child2}</new>