(: Name: preceding-sibling-10 :)
(: Description: Evaluation of the preceding-sibling axis that is part of an "node after" expression with different operands (returns false). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[13]) >> exactly-one($input-context1/works[1]/employee[2]/preceding-sibling::employee)
