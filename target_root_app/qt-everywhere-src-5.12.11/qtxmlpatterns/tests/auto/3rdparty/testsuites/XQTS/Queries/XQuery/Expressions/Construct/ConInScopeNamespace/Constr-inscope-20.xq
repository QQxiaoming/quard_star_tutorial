(: Name: Constr-inscope-20 :)
(: Written by: Andreas Behm :)
(: Description: copy element with same default namespace :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new xmlns="http://www.example.com/parent4">{$input-context//*:child4}</new>