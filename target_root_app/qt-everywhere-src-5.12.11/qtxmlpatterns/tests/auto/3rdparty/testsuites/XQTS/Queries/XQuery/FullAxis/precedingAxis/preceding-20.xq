(: Name: preceding-20 :)
(: Description: Evaluation of the preceding axis that used as part of the deep-equal-function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:deep-equal($input-context1/works[1]/employee[12]/preceding::employee,$input-context1/works[1]/employee[12]/preceding::employee)