(: Name: following-10 :)
(: Description: Evaluation of the following axis that is part of an "node after" expression with different operands (returns false). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[12]) >> exactly-one($input-context1/works[1]/employee[12]/following::employee)
