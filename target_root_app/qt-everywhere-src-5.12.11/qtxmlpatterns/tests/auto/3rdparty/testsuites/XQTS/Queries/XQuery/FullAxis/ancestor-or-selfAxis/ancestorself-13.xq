(: Name: ancestorself-13 :)
(: Description: Evaluation of the ancestor-or-self axis that is part of an "intersect" operation. Both operands are the same. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/overtime/day/ancestor-or-self::employee) intersect ($input-context1/works/employee[12]/overtime/day/ancestor-or-self::employee)