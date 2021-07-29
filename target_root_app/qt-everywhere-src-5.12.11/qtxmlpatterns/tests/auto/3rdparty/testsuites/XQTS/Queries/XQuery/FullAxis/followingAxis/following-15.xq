(: Name: following-15 :)
(: Description: Evaluation of the following axis that is part of an "except" operation. Both operands are different. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]/employee[12]/overtime/day) except ($input-context1/works[1]/employee[12]/overtime/day[1]/following::day)