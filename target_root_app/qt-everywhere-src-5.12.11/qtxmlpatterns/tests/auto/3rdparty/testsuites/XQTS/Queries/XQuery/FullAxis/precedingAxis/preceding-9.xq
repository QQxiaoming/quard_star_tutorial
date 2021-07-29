(: Name: preceding-9 :)
(: Description: Evaluation of the preceding axis that is part of an "node after" expression with both operands the same (returns false). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[2]/preceding::employee) >> exactly-one($input-context1/works[1]/employee[2]/preceding::employee)
