(: Name: ancestor-15 :)
(: Description: Evaluation of the ancestor axis that is part of an "except" operation. Both operands are different. :)
(: Uses fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/overtime/day[ancestor::overtime]) except ($input-context1/works/employee[12]/overtime/day[1])