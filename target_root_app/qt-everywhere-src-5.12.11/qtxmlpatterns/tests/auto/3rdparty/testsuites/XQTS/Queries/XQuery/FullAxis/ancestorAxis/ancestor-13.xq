(: Name: ancestor-13 :)
(: Description: Evaluation of the ancestor axis that is part of an "intersect" operation. Both operands are the same. :)
(: Uses fn:count to avoid empty file :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/overtime/day/ancestor::employee) intersect ($input-context1/works/employee[12]/overtime/day/ancestor::employee)