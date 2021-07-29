(: Name: following-12 :)
(: Description: Evaluation of the following axis that is part of an "union" operation. Both operands are different :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works/employee[12]/*/day[1]/following::day) | ($input-context1/works/employee[12]/*/day[1])