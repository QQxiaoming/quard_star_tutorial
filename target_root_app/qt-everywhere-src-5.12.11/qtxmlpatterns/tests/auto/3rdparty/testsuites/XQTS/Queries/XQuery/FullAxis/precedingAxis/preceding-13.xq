(: Name: preceding-13 :)
(: Description: Evaluation of the preceding axis that is part of an "intersect" operation. Both operands are the same. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]/employee[12]/overtime[1]/day[2]/preceding::day) intersect ($input-context1/works[1]/employee[12]/overtime[1]/day[2]/preceding::day)