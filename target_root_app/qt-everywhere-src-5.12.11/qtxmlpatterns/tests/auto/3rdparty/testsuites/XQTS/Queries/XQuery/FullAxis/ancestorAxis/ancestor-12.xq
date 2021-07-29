(: Name: ancestor-12 :)
(: Description: Evaluation of the ancestor axis that is part of an "union" operation. Both operands are different :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/*/day[1]/ancestor::overtime) | ($input-context1/works/employee[12]/*/day[2]/ancestor::overtime)