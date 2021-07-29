(: Name: following-4 :)
(: Description: Evaluation of the following axis that is part of an "is" expression (return false). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[12]/following::employee) is exactly-one($input-context1/works[1]/employee[12])
