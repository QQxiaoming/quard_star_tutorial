(: Name: ancestorself-5 :)
(: Description: Evaluation of the ancestor-or-self axis that is part of an "node before" expression (return true). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works/employee[1]/ancestor-or-self::works) << exactly-one($input-context1/works/employee[1])
