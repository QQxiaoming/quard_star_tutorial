(: Name: Constr-inscope-19 :)
(: Written by: Andreas Behm :)
(: Description: copy element with different default namespace :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new xmlns="http://www.example.com">{$input-context//*:child4}</new>