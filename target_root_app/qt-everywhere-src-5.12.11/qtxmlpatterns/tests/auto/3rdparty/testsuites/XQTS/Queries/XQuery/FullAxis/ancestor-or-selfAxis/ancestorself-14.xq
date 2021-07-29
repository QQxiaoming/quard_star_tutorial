(: Name: ancestorself-14 :)
(: Description: Evaluation of the ancestor-self axis that is part of an "except" operation. Both operands are the same. :)
(: Uses fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(($input-context1/works/employee[12]/overtime/day[ancestor-or-self::overtime]) except ($input-context1/works/employee[12]/overtime/day[ancestor-or-self::overtime]))