(: Name: ancestor-19 :)
(: Description: Evaluation of the ancestor axis that is part of a boolean expression ("or" and fn:false()). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/overtime/day[ancestor::overtime]) or fn:false()