(: Name: Constr-inscope-18 :)
(: Written by: Andreas Behm :)
(: Description: copy element with same prefix :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new xmlns:foo="http://www.example.com/parent1">{$input-context//*:child1}</new>