(: Name: ancestorself-15 :)
(: Description: Evaluation of the ancestor-or-self axis that is part of an "except" operation. Both operands are different. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/overtime/day[ancestor-or-self::overtime]) except ($input-context1/works/employee[12]/overtime/day[1])