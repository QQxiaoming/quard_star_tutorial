(: Name: preceding-6 :)
(: Description: Evaluation of the preceding axis that is part of an "node before" expression and both operands are the same (return false). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[2]/preceding::employee) << exactly-one($input-context1/works[1]/employee[2]/preceding::employee)
