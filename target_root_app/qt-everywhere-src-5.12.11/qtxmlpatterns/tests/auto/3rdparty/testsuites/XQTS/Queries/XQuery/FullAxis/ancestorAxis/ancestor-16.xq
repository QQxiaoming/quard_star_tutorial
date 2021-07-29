(: Name: ancestor-16 :)
(: Description: Evaluation of the ancestor axis that is part of a boolean expression ("and" and fn:true(). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/overtime/day[ancestor::overtime]) and fn:true()