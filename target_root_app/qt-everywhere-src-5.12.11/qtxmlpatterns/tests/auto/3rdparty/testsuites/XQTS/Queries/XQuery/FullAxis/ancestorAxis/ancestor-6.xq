(: Name: ancestor-6 :)
(: Description: Evaluation of the ancestor axis that is part of an "node before" expression and both operands are the same (return false). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works/employee[1]/ancestor::works) << exactly-one($input-context1/works/employee[1]/ancestor::works)
