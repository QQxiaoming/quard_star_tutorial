(: Name: preceding-3 :)
(: Description: Evaluation of the preceding axis that is part of an "is" expression (return true). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works/employee[2]/preceding::employee) is exactly-one($input-context1/works/employee[1])
