(: Name: following-5 :)
(: Description: Evaluation of the following axis that is part of an "node before" expression (return true). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]/employee[11]/following::employee[1]) << ($input-context1/works[1]/employee[13])