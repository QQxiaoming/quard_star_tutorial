(: Name: ancestorself-19 :)
(: Description: Evaluation of the ancestor-or-self axis that is part of a boolean expression ("or" and fn:false()). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/overtime/day[ancestor-or-self::overtime]) or fn:false()