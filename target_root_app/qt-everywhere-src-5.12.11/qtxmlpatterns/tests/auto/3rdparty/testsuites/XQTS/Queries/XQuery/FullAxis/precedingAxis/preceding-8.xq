(: Name: preceding-8 :)
(: Description: Evaluation of the preceding axis that is part of a "node after" expression (returns true). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[13]) >> exactly-one($input-context1/works[1]/employee[12]/overtime[1]/day[2]/preceding::day)
