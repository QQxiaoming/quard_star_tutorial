(: Name: combined-errors-4 :)
(: Description: Evaluates simple full axis feature (preceding axis) to generate error code if feature not supported. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]/employee[2]/preceding::employee) 